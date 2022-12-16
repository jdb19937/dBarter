// {HEADER}

#define _TID_CC
#include "bom.h"

bom::tid::tid() : bom::object() {
  acct = NULL;
  command = NULL;
  next = NULL;
}

bom::tid::tid(char *_id) : bom::object(_id) {
  acct = NULL;
  command = NULL;
  next = NULL;
}

bom::tid::tid(const bom::tid& txn) : bom::object(txn) {
  acct = NULL;
  command = NULL;
  next = NULL;
}

bom::tid::~tid() {
  if (acct)
    delete acct;
  if (command)
    delete command;
  if (next)
    delete next;
}

int bom::tid::gen_id() {
  if (id)
    bom_os_free(id);
  if (!(id = (char *)bom_os_malloc(3 + 1 + 8 + 8 + 1))) 
    return -1;
  sprintf(id, "TID:%08X%08X",
   (unsigned int)time(0), (unsigned int)rand());
  return 0;
}

int bom::tid::get(txn_t *txn, u_int32_t attrs) {
  if (attrs & TID_ATTR_COMMAND) {
    if (command)
      bom_os_free(command);
    if (int result = get_command(txn, &command))
      return result;
  }
  if (attrs & TID_ATTR_ACCT) {
    if (acct)
      delete acct;
    acct = new bom::acct;
    if (int result = get_acct(txn, acct))
      return result;
  }
  
  return 0;
}

int bom::tid::set(txn_t *txn, u_int32_t attrs) {
  if (attrs & TID_ATTR_COMMAND)
    if (int result = set_command(txn, command))
      return result;
  if (attrs & TID_ATTR_ACCT)
    if (int result = set_acct(txn, *acct))
      return result;
  
  return 0;
}

int bom::tid::exists(txn_t *txn, char *exists) {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_TID_ACCT);

  string_to_dbt(id, &k);
  dbt_init(&v);
  result = db->get(db, txn, &k, &v, 0);
  dbt_clear(&k);
  dbt_clear(&v);

  *exists = 0;
  if (result == DB_NOTFOUND)
    return 0;
  if (result)
    return result;

  *exists = 1;
  return 0;
}

int bom::tid::get_command(txn_t *txn, char **_command) {
  DB *db;
  DBT k, v;
  int result;
  
  db = db_insure_open(DB_TID_COMMAND);

  string_to_dbt(id, &k);
  dbt_init(&v);

  result = db->get(db, txn, &k, &v, 0);
  dbt_clear(&k);

  if (result) {
    dbt_clear(&v);
    return result;
  }
  
  *_command = bom_os_strdup((char *)v.data);
  dbt_clear(&v);

  return 0;
}

int bom::tid::set_command(txn_t *txn, char *_command) {
  DB *db;
  DBT k, v;
  int result;
  
  db = db_insure_open(DB_TID_COMMAND);

  string_to_dbt(id, &k);
  string_to_dbt(_command, &v);

  result = db->put(db, txn, &k, &v, 0);
  dbt_clear(&k);
  dbt_clear(&v);

  if (result)
    return result;
  
  return 0;
}

int bom::tid::del_command(txn_t *txn) {
  DB *db;
  DBT k;
  int result;
  
  db = db_insure_open(DB_TID_COMMAND);

  string_to_dbt(id, &k);
  result = db->del(db, txn, &k, 0);
  dbt_clear(&k);

  if (result == DB_NOTFOUND)
    return 0;
  if (result)
    return result;
  
  return 0;
}

int bom::tid::get_acct(txn_t *txn, bom::acct *_acct) {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_TID_ACCT);

  string_to_dbt(id, &k);
  dbt_init(&v);

  result = db->get(db, txn, &k, &v, 0);
  dbt_clear(&k);

  if (result)
    return result;

  _acct->set_id((char *)v.data);
  dbt_clear(&v);

  return 0;
}

int bom::tid::set_acct(txn_t *txn, const bom::acct& _acct) {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_TID_ACCT);

  string_to_dbt(id, &k);
  string_to_dbt(_acct.id, &v);
  result = db->put(db, txn, &k, &v, 0);
  dbt_clear(&k);
  dbt_clear(&v);

  if (result)
    return result;
  
  return 0;
}

int bom::tid::del_acct(txn_t *txn) {
  DBT k;
  DB *db;
  int result;

  string_to_dbt(id, &k);
  db = db_insure_open(DB_TID_ACCT);

  result = db->del(db, txn, &k, 0);
  dbt_clear(&k);

  if (result == DB_NOTFOUND) {
    return 0;
  } else if (result) {
    return result;
  }

  return 0;
}

int bom::tid::destroy(txn_t *txn) {
  int result;

  if (result = del_command(txn))
    return result;
  if (result = del_acct(txn))
    return result;

  return 0;
}

