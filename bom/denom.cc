// {HEADER}

#define _DENOM_CC
#include "bom.h"

bom::denom::denom() : bom::object() {
  _promise = NULL;
  _issuer = NULL;
  next = NULL;
}

bom::denom::denom(char *_id) : bom::object(_id) {
  _promise = NULL;
  _issuer = NULL;
  next = NULL;
}

bom::denom::denom(const bom::denom& denom) : bom::object(denom) {
  _promise = NULL;
  _issuer = NULL;
  next = NULL;
}

bom::denom::~denom() {
  if (_promise)
    delete _promise;
  if (_issuer)
    delete _issuer;
  if (next)
    delete next;
}

int bom::denom::get(txn_t *txn, u_int32_t attrs) {
  if (attrs & DENOM_ATTR_ISSUER) {
    if (_issuer)
      delete _issuer;
    _issuer = new bom::acct;
    if (int result = get_issuer(txn, _issuer))
      return result;
  }
  if (attrs & DENOM_ATTR_PROMISE) {
    if (_promise)
      delete _promise;
    _promise = NULL;
    if (int result = get_promise(txn, &_promise))
      return result;
  }
  
  return 0;
}

int bom::denom::set(txn_t *txn, u_int32_t attrs) {
  if (attrs & DENOM_ATTR_ISSUER)
    if (int result = set_issuer(txn, *_issuer))
      return result;
  if (attrs & DENOM_ATTR_PROMISE)
    if (int result = set_promise(txn, _promise))
      return result;
  
  return 0;
}

int bom::denom::exists(txn_t *txn, char *exists) const {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_DENOM_ISSUER);

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

int bom::denom::destroy(txn_t *txn) const {
  int result;

  bom::event event;
  if (result = event_new_denom_destroy(txn, &event, *this))
    return result;

  if (result = del_issuer(txn))
    return result;
  if (result = del_promise(txn))
    return result;

  return 0;
}

int bom::denom::get_promise(txn_t *txn, char **promise) const {
  DB *db;
  DBT k, v;
  int result;
  
  db = db_insure_open(DB_DENOM_PROMISE);

  string_to_dbt(id, &k);
  dbt_init(&v);

  result = db->get(db, txn, &k, &v, 0);
  dbt_clear(&k);

  if (result) {
    dbt_clear(&v);
    return result;
  }
  
  *promise = bom_os_strdup((char *)v.data);
  dbt_clear(&v);

  return 0;
}

int bom::denom::set_promise(txn_t *txn, char *promise) const {
  DB *db;
  DBT k, v;
  int result;
  
  db = db_insure_open(DB_DENOM_PROMISE);

  string_to_dbt(id, &k);
  string_to_dbt(promise, &v);

  result = db->put(db, txn, &k, &v, 0);
  dbt_clear(&k);
  dbt_clear(&v);

  if (result)
    return result;
  
  return 0;
}

int bom::denom::del_promise(txn_t *txn) const {
  DB *db;
  DBT k;
  int result;
  
  db = db_insure_open(DB_DENOM_PROMISE);

  string_to_dbt(id, &k);
  result = db->del(db, txn, &k, 0);
  dbt_clear(&k);

  if (result == DB_NOTFOUND)
    return 0;
  if (result)
    return result;
  
  return 0;
}

int bom::denom::get_issuer(txn_t *txn, bom::acct *issuer) const {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_DENOM_ISSUER);

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

int bom::denom::set_issuer(txn_t *txn, const bom::acct& issuer) const {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_DENOM_ISSUER);
  DB *dbr = db_insure_open(DB_ISSUER_DENOM);
  DBC *c;

  string_to_dbt(id, &k);
  string_to_dbt(issuer.id, &v);
  result = db->put(db, txn, &k, &v, 0);
  dbt_clear(&k);
  dbt_clear(&v);

  if (result)
    return result;
  
  result = dbr->cursor(dbr, txn, &c, 0);
  if (result)
    return result;

  string_to_dbt(issuer.id, &k);
  string_to_dbt(id, &v);
  
  result = c->c_get(c, &k, &v, DB_GET_BOTH);
  
  if (result == 0) {
    dbt_clear(&k);
    dbt_clear(&v);
    if (result = c->c_close(c))
      return result;
    return 0;
  } else if (result != DB_NOTFOUND) {
    dbt_clear(&k);
    dbt_clear(&v);
    c->c_close(c);
    return result;
  }

  result = c->c_put(c, &k, &v, DB_KEYFIRST);
  dbt_clear(&k);
  dbt_clear(&v);

  if (result) {
    c->c_close(c);
    return result;
  }
  
  if (result = c->c_close(c))
    return result;
  
  return 0;
}

int bom::denom::del_issuer(txn_t *txn) const {
  DBT k, v;
  DB *db;
  DBC *c;
  int result;

  string_to_dbt(id, &k);
  dbt_init(&v);
  
  db = db_insure_open(DB_DENOM_ISSUER);

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

  db = db_insure_open(DB_ISSUER_DENOM);
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

int get_denom_list(txn_t *txn, bom::denom **dl) {
  DB *db;
  DBC *c;
  DBT k, v;
  int result;
  bom::denom *d;
  
  *dl = NULL;

  db = db_insure_open(DB_DENOM_ISSUER);
  if (result = db->cursor(db, txn, &c, 0))
    return result;
  
  dbt_init(&k);
  dbt_init(&v);
  
  while (1) {
    result = c->c_get(c, &k, &v, DB_NEXT);
    if (result == DB_NOTFOUND) {
      if (result = c->c_close(c)) {
	if (dl) {
	  delete dl;
	  *dl = NULL;
	}
        return result;
      }
      return 0;
    }

    if (result) {
      dbt_clear(&k);
      dbt_clear(&v);
      c->c_close(c);
      return result;
    }
  
    d = new bom::denom((char *)k.data);
    d->next = *dl;
    *dl = d;

    dbt_clear(&k);
    dbt_clear(&v);
  }
}

int search_denom_list(txn_t *txn, char *pattern, bom::denom **dl) {
  DB *db;
  DBC *c;
  DBT k, v;
  int result;
  bom::denom *d;
  
  *dl = NULL;

  db = db_insure_open(DB_DENOM_PROMISE);
  if (result = db->cursor(db, txn, &c, 0))
    return result;
  
  dbt_init(&k);
  dbt_init(&v);
  
  while (1) {
    result = c->c_get(c, &k, &v, DB_NEXT);
    if (result == DB_NOTFOUND) {
      if (result = c->c_close(c)) {
	if (dl) {
	  delete dl;
	  *dl = NULL;
	}
        return result;
      }
      return 0;
    }

    if (result) {
      dbt_clear(&k);
      dbt_clear(&v);
      c->c_close(c);
      return result;
    }
    
    if (!strstr((char *)v.data, pattern)) {
      dbt_clear(&k);
      dbt_clear(&v);
      continue;
    }
  
    d = new bom::denom((char *)k.data);
    d->next = *dl;
    *dl = d;

    dbt_clear(&k);
    dbt_clear(&v);
  }
}

int denom_list_to_string(bom::denom *ol, char **sp) {
  int length = 0, count = 0;

  if (ol) {
    for (bom::denom *o = ol; o; o = o->next)
      length += strlen(o->id) + 1;

    *sp = (char *)bom_os_malloc(length);

    bom::denom *o = ol;
    count += sprintf(*sp + count, "%s", o->id);
    for (o = o->next; o; o = o->next)
      count += sprintf(*sp + count, " %s", o->id);
  } else {
    *sp = (char *)bom_os_malloc(1);
    **sp = '\0';
  }
  
  return 0;
}

int string_to_denom_list(char *_s, bom::denom **olp) {
  char *s, *p, *q;
  int result;
  
  q = s = bom_os_strdup(_s);
  *olp = NULL;

  while (p = strchr(q, ' ')) {
    *p++ = '\0';

    bom::denom *op = new bom::denom;
    op->set_id(q);
    op->next = *olp;
    *olp = op;
    
    while (isspace(*p))
      p++;
    q = p;
  }

  if (*q) {
    bom::denom *op = new bom::denom;
    op->set_id(q);
    op->next = *olp;
    *olp = op;
  }
  
  bom_os_free(s);
  return 0;
}
