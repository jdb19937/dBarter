// {HEADER}

#define _DB_CC
#include "bom.h"

DB_ENV db_env;
DB_INFO info_hash, info_hash_dup, info_btree,
 info_btree_dup, info_btree_dupsort, info_recno, info_offer_index;
DB_TXNMGR* db_txnmgr = NULL;
DB_LOCKTAB* db_locktab = NULL;
static long extra_flags = 0;
static int _threaded = 0;
int lock_fd;

struct db_info_t db_info[] = {
  { "config.db", 0, DB_HASH, &info_hash },
  { "acct_public.db", 0, DB_HASH, &info_hash },
  { "acct_secret.db", 0, DB_HASH, &info_hash },
  { "note_challenge.db", 0, DB_HASH, &info_hash },
  { "note_issuer.db", 0, DB_HASH, &info_hash },
  { "acct_note.db", 0, DB_HASH, &info_hash_dup },
  { "acct_modulus.db", 0, DB_HASH, &info_hash },
  { "note_value.db", 0, DB_BTREE, &info_btree },
  { "acct_value.db", 0, DB_BTREE, &info_btree },
  { "denom_type.db", 0, DB_HASH, &info_hash },
  { "denom_promise.db", 0, DB_HASH, &info_hash },
  { "denom_issuer.db", 0, DB_HASH, &info_hash },
  { "issuer_denom.db", 0, DB_HASH, &info_hash_dup },
  { "acct_flags.db", 0, DB_HASH, &info_hash },
  { "event_type.db", 0, DB_RECNO, &info_recno },
  { "event_data.db", 0, DB_RECNO, &info_recno },
  { "event_acct.db", 0, DB_RECNO, &info_recno },
  { "acct_event.db", 0, DB_HASH, &info_hash_dup },
  { "tid_acct.db", 0, DB_HASH, &info_hash },
  { "tid_command.db", 0, DB_HASH, &info_hash },
  { "acct_promise.db", 0, DB_HASH, &info_hash },

  { "offer_index.db", 0, DB_HASH, &info_offer_index },
  { "offer_bid_value.db", 0, DB_HASH, &info_hash },
  { "offer_ask_value.db", 0, DB_HASH, &info_hash },
  { "offer_issuer.db", 0, DB_HASH, &info_hash },
  { "acct_offer.db", 0, DB_HASH, &info_hash_dup },

  { "", 0, DB_HASH, &info_hash }
};

int dup_compare_offer_index(const DBT *a, const DBT *b) {
  dmath::bq aq, bq;

  string_to_bq((char *)a->data + strlen((char *)a->data) + 1, &aq);
  string_to_bq((char *)b->data + strlen((char *)b->data) + 1, &bq);
  return mpq_cmp(aq, bq);
}

void db_init(bool threaded) {
  int flags;
  _threaded = threaded ? DB_THREAD : 0;

  memset(&info_recno, 0, sizeof(info_recno));

  memset(&info_hash, 0, sizeof(info_hash));
  info_hash.h_nelem = 1 << 24;

  memset(&info_hash_dup, 0, sizeof(info_hash_dup));
  info_hash_dup.h_nelem = 1 << 24;
  info_hash_dup.flags = DB_DUP;

  memset(&info_btree, 0, sizeof(info_btree));

  memset(&info_btree_dup, 0, sizeof(info_btree_dup));
  info_btree_dup.flags = DB_DUP;

  memset(&info_btree_dupsort, 0, sizeof(info_btree_dupsort));
  info_btree_dupsort.flags = DB_DUP | DB_DUPSORT;

  memset(&info_offer_index, 0, sizeof(DB_INFO));
  info_offer_index.flags = DB_DUP | DB_DUPSORT;
  info_offer_index.dup_compare = dup_compare_offer_index;

  memset(&db_env, 0, sizeof(DB_ENV));
  db_env.lk_detect = DB_LOCK_OLDEST;

  {
    char *lock_fn = (char *)bom_os_malloc(strlen(bom_db_directory) + 256);
    sprintf(lock_fn, "%s/.lock", bom_db_directory);
    if ((lock_fd = open(lock_fn, O_CREAT | O_RDWR, 0600)) < 0) {
      errorf("%s: cannot open\n", lock_fn);
      exit(1);
    }
    
    if (flock(lock_fd, LOCK_EX | LOCK_NB)) {
      errorf("%s: cannot acquire lock\n", lock_fn);
      exit(1);
    }

    bom_os_free(lock_fn);
  }

  flags =
   DB_CREATE | DB_INIT_TXN |
   DB_INIT_LOCK | DB_INIT_LOG | DB_RECOVER | _threaded;

  if (int result = db_appinit(bom_db_directory, NULL, &db_env, flags)) {
    errorf("%s: error opening db\n", bom_db_directory);
    exit(1);
  }

  db_txnmgr = db_env.tx_info;
  db_locktab = db_env.lk_info;
  
  //txn_checkpoint(db_txnmgr, 0, 0);
}

void db_exit() {
  for (int i = 0; *db_info[i].name; i++)  
    if (db_info[i].db) {
      db_info[i].db->close(db_info[i].db, 0);
      db_info[i].db = NULL;
    }

  db_appexit(&db_env);

  flock(lock_fd, LOCK_UN);
}

DB* db_insure_open(int dn) {
  int result;
  
  if (!db_info[dn].db) {
    result = db_open(db_info[dn].name, db_info[dn].type, 
     DB_CREATE | _threaded, 0600, &db_env, db_info[dn].info, &db_info[dn].db);
    if (result) {
      errorf("cannot open db: %s\n", db_info[dn].name);
      exit(1);
    }
  }
  
  return db_info[dn].db;
}

void db_close(int dn) {
  db_info[dn].db->close(db_info[dn].db, 0);
  db_info[dn].db = NULL;
}

int bom_db_get(char *name, DB_TXN *txn, char *key, char **value) {
  DB *db;
  int result;
  DBT k, v;

  if (result = db_open(name, DB_HASH,
   DB_CREATE | _threaded, 0600, &db_env, &info_hash, &db))
    return result;
  
  string_to_dbt(key, &k);
  dbt_init(&v);
  
  if (result = db->get(db, txn, &k, &v, 0)) {
    dbt_clear(&k);
    return result;
  }

  dbt_clear(&k);
  if (result = db->close(db, 0)) {
    dbt_clear(&v);
    return result;
  }
  
  dbt_to_string(v, value);
  dbt_clear(&v);

  return 0;
}

int bom_db_put(char *name, DB_TXN *txn, char *key, char *value) {
  DB *db;
  int result;
  DBT k, v;

  if (result = db_open(name, DB_HASH,
   DB_CREATE | _threaded, 0600, &db_env, &info_hash, &db))
    return result;
  
  string_to_dbt(key, &k);
  string_to_dbt(value, &v);
  
  if (result = db->put(db, txn, &k, &v, 0)) {
    dbt_clear(&k);
    dbt_clear(&v);
    return result;
  }

  dbt_clear(&k);
  dbt_clear(&v);

  if (result = db->close(db, 0))
    return result;
  
  return 0;
}

int bom_db_del(char *name, DB_TXN *txn, char *key) {
  DB *db;
  int result;
  DBT k;

  if (result = db_open(name, DB_HASH,
   DB_CREATE | _threaded, 0600, &db_env, &info_hash, &db))
    return result;
  
  string_to_dbt(key, &k);
  
  result = db->del(db, txn, &k, 0);
  if (result && result != DB_NOTFOUND) {
    dbt_clear(&k);
    return result;
  }

  dbt_clear(&k);

  if (result = db->close(db, 0))
    return result;
  
  return 0;
}

int dbt_init(DBT *dp) {
  memset(dp, 0, sizeof(DBT));
  dp->flags = DB_DBT_MALLOC;
  return 0;
}

int dbt_clear(DBT *dp) {
  if (dp->data)
    bom_os_free(dp->data);
  dp->data = NULL;
  dp->size = 0;
  return 0;
}

int string_to_dbt(char *s, DBT *dp) {
  memset(dp, 0, sizeof(DBT));
  if (!(dp->data = bom_os_strdup(s)))
    return -1;
  dp->size = strlen(s) + 1;
  dp->flags = DB_DBT_MALLOC;
  return 0;
}

int dbt_to_string(const DBT &d, char **sp) {
  if (!(*sp = bom_os_strdup((char *)d.data)))
    return -1;
  return 0;
}

int int32_to_dbt(u_int32_t i, DBT *dp) {
  memset(dp, 0, sizeof(DBT));
  if (!(dp->data = bom_os_malloc(sizeof(u_int32_t))))
    return -1;
  *(u_int32_t *)dp->data = i;
  dp->size = sizeof(u_int32_t);
  dp->flags = DB_DBT_MALLOC;
  return 0;
}

int dbt_to_int32(const DBT &d, u_int32_t *ip) {
  *ip = *(u_int32_t *)d.data;
  return 0;
}
