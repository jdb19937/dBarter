// {HEADER}

#define _NOTE_CC
#include "bom.h"

bom::note::note() : bom::object() {
  _issuer = NULL;
  _challenge = NULL;
  _values = NULL; 
  next = NULL;
}

bom::note::note(char *_id) : bom::object(_id) {
  _issuer = NULL;
  _challenge = NULL;
  _values = NULL; 
  next = NULL;
}

bom::note::note(const bom::note& note) : bom::object(note) {
  _issuer = NULL;
  _challenge = NULL;
  _values = NULL; 
  next = NULL;
}

bom::note::~note() {
  if (_issuer)
    delete _issuer;
  if (_challenge)
    delete _challenge;
  if (_values)
    delete _values;
  if (next)
    delete next;
}

int bom::note::get(txn_t *txn, u_int32_t attrs) {
  if (attrs & NOTE_ATTR_ISSUER) {
    if (_issuer)
      delete _issuer;
    _issuer = new bom::acct;
    if (int result = get_issuer(txn, _issuer))
      return result;
  }
  if (attrs & NOTE_ATTR_CHALLENGE) {
    if (_challenge)
      delete _challenge;
    _challenge = new dmath::bz;
    if (int result = get_challenge(txn, _challenge))
      return result;
  }
  if (attrs & NOTE_ATTR_VALUES) {
    if (_values)
      delete _values;
    _values = NULL;
    if (int result = get_portfolio(txn, &_values))
      return result;
  }
  
  return 0;
}

int bom::note::set(txn_t *txn, u_int32_t attrs) {
  if (attrs & NOTE_ATTR_ISSUER)
    if (int result = set_issuer(txn, *_issuer))
      return result;
  if (attrs & NOTE_ATTR_CHALLENGE)
    if (int result = set_challenge(txn, *_challenge))
      return result;
  if (attrs & NOTE_ATTR_VALUES)
    for (bom::value *v = _values; v; v = v->next)
      if (int result = set_value(txn, *v))
        return result;

  return 0;
}

int bom::note::exists(txn_t *txn, char *exists) {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_NOTE_ISSUER);

  *exists = 0;

  string_to_dbt(id, &k);
  dbt_init(&v);

  result = db->get(db, txn, &k, &v, 0);

  dbt_clear(&k);

  if (result == DB_NOTFOUND)
    return 0;
  if (result)
    return result;

  dbt_clear(&v);

  *exists = 1;
  return 0;
}

int bom::note::get_issuer(txn_t *txn, bom::acct *issuer) {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_NOTE_ISSUER);

  string_to_dbt(id, &k);
  dbt_init(&v);

  result = db->get(db, txn, &k, &v, 0);
  dbt_clear(&k);

  if (result)
    return result;

  issuer->set_id((char *)v.data);
  dbt_clear(&v);

  return 0;
}

int bom::note::set_issuer(txn_t *txn, const bom::acct& issuer) {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_NOTE_ISSUER);
  DB *dbr = db_insure_open(DB_ACCT_NOTE);

  string_to_dbt(id, &k);
  string_to_dbt(issuer.id, &v);
  result = db->put(db, txn, &k, &v, 0);
  dbt_clear(&k);
  dbt_clear(&v);
  if (result)
    return result;
  
  string_to_dbt(issuer.id, &k);
  string_to_dbt(id, &v);
  
  result = dbr->get(dbr, txn, &k, &v, DB_GET_BOTH);
  if (result == 0) {
    dbt_clear(&k);
    dbt_clear(&v);
    return 0;
  } else if (result != DB_NOTFOUND) {
    dbt_clear(&k);
    dbt_clear(&v);
    return result;
  }

  result = dbr->put(dbr, txn, &k, &v, 0);
  dbt_clear(&k);
  dbt_clear(&v);

  if (result)
    return result;
  
  return 0;
}

int bom::note::del_issuer(txn_t *txn) {
  DBT k, v;
  DB *db;
  DBC *c;
  int result;

  string_to_dbt(id, &k);
  dbt_init(&v);
  
  db = db_insure_open(DB_NOTE_ISSUER);

  result = db->get(db, txn, &k, &v, 0);
  if (result == DB_NOTFOUND) {
    dbt_clear(&k);
    return 0;
  }
  if (result) {
    dbt_clear(&k);
    return result;
  }

  if (result = db->del(db, txn, &k, 0)) {
    dbt_clear(&k);
    dbt_clear(&v);
    return result;
  }

  db = db_insure_open(DB_ACCT_NOTE);
  if (result = db->cursor(db, txn, &c, 0)) {
    dbt_clear(&k);
    dbt_clear(&v);
    return result;
  }
  result = c->c_get(c, &v, &k, DB_GET_BOTH);

  dbt_clear(&k);
  dbt_clear(&v);

  if (result == DB_NOTFOUND) {
    if (result = c->c_close(c))
      return result;
    return 0;
  }
  if (result) {
    dbt_clear(&k);
    dbt_clear(&v);
    c->c_close(c);
    return result;
  }
  
  if (result = c->c_del(c, 0)) {
    c->c_close(c);
    return result;
  }
  if (result = c->c_close(c))
    return result;
  
  return 0;
}

int bom::note::get_challenge(txn_t *txn, dmath::bz* c) {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_NOTE_CHALLENGE);

  string_to_dbt(id, &k);
  dbt_init(&v);

  result = db->get(db, txn, &k, &v, 0);

  dbt_clear(&k);
  if (result)
    return result;

  dbt_to_bz(v, c);
  dbt_clear(&v);
  
  return 0;
}

int bom::note::set_challenge(txn_t *txn, const dmath::bz& c) {
  DBT k, v;
  int result;
  DB* db = db_insure_open(DB_NOTE_CHALLENGE);
  
  string_to_dbt(id, &k);
  bz_to_dbt(c, &v);
  result = db->put(db, txn, &k, &v, 0);
  dbt_clear(&k);
  dbt_clear(&v);

  if (result)
    return result;

  return 0;
}

int bom::note::del_challenge(txn_t *txn) {
  DB *db;
  DBT k;
  int result;
  
  string_to_dbt(id, &k);

  db = db_insure_open(DB_NOTE_CHALLENGE);
  result = db->del(db, txn, &k, 0);
  dbt_clear(&k);
  if (result == DB_NOTFOUND) 
    return 0;
  if (result)
    return result;
  
  return 0;
}

int bom::note::get_value(txn_t *txn, bom::value *val) {
  DBT k, v;
  int result, us, ds;
  DB *db = db_insure_open(DB_NOTE_VALUE);

  memset(&k, 0, sizeof(k));
  k.data = (char *)
   bom_os_malloc(k.size = (us = 1 + strlen(id)) + (ds = 1 + strlen(val->denom.id)));
  if (!k.data)
    return -1;
  memcpy((char *)k.data, id, us);
  memcpy((char *)k.data + us, val->denom.id, ds);
  k.flags = DB_DBT_MALLOC;
  
  dbt_init(&v);
  
  result = db->get(db, txn, &k, &v, 0);
  
  dbt_clear(&k);

  if (result) {
    if (result == DB_NOTFOUND) {
      val->quant = 0;
      return 0;
    }
    return result;
  }
  
  dbt_to_bq(v, &val->quant);
  dbt_clear(&v);
  
  return 0;
}

int bom::note::set_value(txn_t *txn, const bom::value &val) {
  DBT k, v;
  int result, us, ds;
  DB *db = db_insure_open(DB_NOTE_VALUE);

  memset(&k, 0, sizeof(k));
  k.data = (char *)
   bom_os_malloc(k.size = (us = 1 + strlen(id)) + (ds = 1 + strlen(val.denom.id)));
  if (!k.data)
    return -1;
  memcpy((char *)k.data, id, us);
  memcpy((char *)k.data + us, val.denom.id, ds);
  k.flags = DB_DBT_MALLOC;
  
  bq_to_dbt(val.quant, &v);
  
  result = db->put(db, txn, &k, &v, 0);
  
  dbt_clear(&k);
  dbt_clear(&v);

  if (result)
    return result;

  return 0;
}

int bom::note::del_value(txn_t *txn, const bom::value &val) {
  DBT k;
  int result, us, ds;
  DB *db = db_insure_open(DB_NOTE_VALUE);

  dbt_init(&k);
  k.data = (char *)
   bom_os_malloc(k.size = (us = 1 + strlen(id)) + (ds = 1 + strlen(val.denom.id)));
  if (!k.data)
    return -1;
  memcpy((char *)k.data, id, us);
  memcpy((char *)k.data + us, val.denom.id, ds);
  
  result = db->del(db, txn, &k, 0);
  
  dbt_clear(&k);

  if (result == DB_NOTFOUND)
    return 0;
  if (result)
    return result;
  
  return 0;
}

int bom::note::get_value_first(DBC *c, bom::value *val, char *exists) {
  DBT k, v;
  int result;
  
  *exists = 0;
  string_to_dbt(id, &k);
  dbt_init(&v);
  
  result = c->c_get(c, &k, &v, DB_SET_RANGE);

  if (result) {
    dbt_clear(&k);
    if (result == DB_NOTFOUND)
      return 0;
    else
      return result;
  }

  if (strcmp((char *)k.data, id)) {
    dbt_clear(&k);
    dbt_clear(&v);
    return 0;
  }
  
  val->denom.set_id((char *)k.data + strlen((char *)k.data) + 1);
  dbt_clear(&k);
  
  dbt_to_bq(v, &val->quant);
  dbt_clear(&v);
  
  *exists = 1;
  return 0;
}

int bom::note::get_value_next(DBC *c, bom::value *val, char *exists) {
  DBT k, v;
  int result;
  
  *exists = 0;
  
  dbt_init(&k);
  dbt_init(&v);

  result = c->c_get(c, &k, &v, DB_NEXT);

  if (result) {
    dbt_clear(&k);
    if (result == DB_NOTFOUND)
      return 0;
    else
      return result;
  }

  if (strcmp((char *)k.data, id)) {
    dbt_clear(&k);
    dbt_clear(&v);
    return 0;
  }
  
  val->denom.set_id((char *)k.data + strlen((char *)k.data) + 1);
  dbt_clear(&k);

  dbt_to_bq(v, &val->quant);
  dbt_clear(&v);
  
  *exists = 1;
  return 0;
}

int bom::note::get_portfolio(txn_t *txn, bom::value **vl) {
  DB *db;
  DBC* c;
  int result;
  char exists = 0;
  bom::value val;
  bom::value *valp;

  *vl = NULL;
  db = db_insure_open(DB_NOTE_VALUE);

  result = db->cursor(db, txn, &c, 0);
  if (result) 
    return result;
 
  result = get_value_first(c, &val, &exists);
  if (result) {
    c->c_close(c);
    return result;
  }

  while (exists) {
    valp = new bom::value(val);
    valp->next = *vl;
    *vl = valp;
   
    result = get_value_next(c, &val, &exists);
    if (result) {
      delete *vl;
      *vl = NULL;
      c->c_close(c);
      return result;
    }
  }

  result = c->c_close(c);
  if (result) {
    delete *vl;
    *vl = NULL;
    return result;
  }

  return 0;
}

int bom::note::set_portfolio(txn_t *txn, bom::value *vl) {
  for (bom::value *v = vl; v; v = v->next) 
    if (int result = set_value(txn, *v))
      return result;
  return 0;
}

int bom::note::del_portfolio(txn_t *txn) {
  DB *db;
  DBC *c;
  int result;
  char exists;
  bom::value value;

  db = db_insure_open(DB_NOTE_VALUE);

  if (result = db->cursor(db, txn, &c, 0))
    return result;

  if (result = get_value_first(c, &value, &exists)) {
    c->c_close(c);
    return result;
  }

  while (exists) {
    if (result = c->c_del(c, 0)) {
      c->c_close(c);
      return result;
    }
    if (result = get_value_next(c, &value, &exists)) {
      c->c_close(c);
      return result;
    }
  }

  if (result = c->c_close(c))
    return result;
  
  return 0;
}

int bom::note::create_challenge(txn_t *txn, dmath::bz* c, char* success) {
  dmath::bz km;
  bom::acct issuer;
  int result;

  *success = 0;

  result = get_issuer(txn, &issuer);
  if (result)
    return result;

  result = issuer.get_modulus(txn, &km);

  if (result)
    return result;

  if (km <= 0)
    return 0;
  
  rsa_challenge(c, km);
  *success = 1;
  
  return 0;
}

int bom::note::verify_reponse(txn_t *txn,
 const dmath::bz& c0, const dmath::bz& r, char *success) {
  dmath::bz km, kp, c1;
  int result;
  bom::acct issuer;
  
  *success = 0;

  result = get_issuer(txn, &issuer);
  if (result)
    return result;

  result = issuer.get_modulus(txn, &km);
  if (result)
    return result;
  
  result = issuer.get_public(txn, &kp);
  if (result)
    return result;

  if (km <= 0)
    return 0;

  rsa_crypt(&c1, r, kp, km);
  *success = (c0 == c1);

  return 0;
}

int bom::note::destroy(txn_t *txn) {
  int result;

  bom::event event;
  if (result = event_new_note_destroy(txn, &event, *this))
    return result;
  
  if (result = del_issuer(txn))
    return result;
  if (result = del_challenge(txn))
    return result;
  if (result = del_portfolio(txn))
    return result;
  
  return 0;
}

int bom::note::gen_id() {
  if (id)
    bom_os_free(id);
  if (!(id = (char *)bom_os_malloc(4 + 1 + 8 + 8 + 1))) 
    return -1;
  sprintf(id, "note:%08X%08X",
   (unsigned int)time(0), (unsigned int)rand());
  return 0;
}

int note_to_string(const bom::note &note, char **sp) {
  if (!note.id)
    return -1;
  if (!(*sp = bom_os_strdup(note.id)))
    return -1;
  return 0;
}

int string_to_note(char *s, bom::note *notep) {
  if (notep->id)
    bom_os_free(notep->id);
  notep->id = NULL;

  if (!(notep->id = bom_os_strdup(s)))
    return -1;
  return 0;
}

int note_list_to_string(bom::note *ol, char **sp) {
  int length = 0, count = 0;

  if (ol) {
    for (bom::note *o = ol; o; o = o->next)
      length += strlen(o->id) + 1;

    *sp = (char *)bom_os_malloc(length);

    bom::note *o = ol;
    count += sprintf(*sp + count, "%s", o->id);
    for (o = o->next; o; o = o->next)
      count += sprintf(*sp + count, " %s", o->id);
  } else {
    *sp = (char *)bom_os_malloc(1);
    **sp = '\0';
  }
  
  return 0;
}

int string_to_note_list(char *_s, bom::note **olp) {
  char *s, *p, *q;
  int result;
  
  q = s = bom_os_strdup(_s);
  *olp = NULL;

  while (p = strchr(q, ' ')) {
    *p++ = '\0';

    bom::note *op = new bom::note;
    op->set_id(q);
    op->next = *olp;
    *olp = op;
    
    while (isspace(*p))
      p++;
    q = p;
  }

  if (*q) {
    bom::note *op = new bom::note;
    op->set_id(q);
    op->next = *olp;
    *olp = op;
  }
  
  bom_os_free(s);
  return 0;
}

