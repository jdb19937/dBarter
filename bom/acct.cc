// {HEADER}

#define _ACCT_CC
#include "bom.h"

char *acct_flag_string[] = {
  "super",
  "issuer",
  "txn",
  ""
};

bom::acct::acct() : bom::object() {
  _public = NULL;
  _secret = NULL;
  _modulus = NULL;
  _promise = NULL;
  _values = NULL;
  _notes = NULL;
  _offers = NULL;
  _events = NULL;
  _flags = (bom::acct_flag)0;
  next = NULL;
}

bom::acct::acct(char *_id) : bom::object(_id) {
  _public = NULL;
  _secret = NULL;
  _modulus = NULL;
  _promise = NULL;
  _values = NULL;
  _notes = NULL;
  _offers = NULL;
  _events = NULL;
  _flags = (bom::acct_flag)0;
  next = NULL;
}

bom::acct::acct(const bom::acct& acct) : bom::object(acct) {
  _public = NULL;
  _secret = NULL;
  _modulus = NULL;
  _promise = NULL;
  _values = NULL;
  _notes = NULL;
  _offers = NULL;
  _events = NULL;
  _flags = (bom::acct_flag)0;
  next = NULL;
}

bom::acct::~acct() {
  if (_public)
    delete _public;
  if (_secret)
    delete _secret;
  if (_modulus)
    delete _modulus;
  if (_promise)
    delete _promise;
  if (_values)
    delete _values;
  if (_notes)
    delete _notes;
  if (_offers)
    delete _offers;
  if (_events)
    delete _events;
  if (next)
    delete next;
}

int bom::acct::get(txn_t *txn, u_int32_t attrs) {
  if (attrs & ACCT_ATTR_MODULUS) {
    if (_modulus)
      delete _modulus;
    _modulus = new dmath::bz;
    if (int result = get_modulus(txn, _modulus))
      return result;
  }
  if (attrs & ACCT_ATTR_PUBLIC) {
    if (_public)
      delete _public;
    _public = new dmath::bz;
    if (int result = get_public(txn, _public))
      return result;
  }
  if (attrs & ACCT_ATTR_SECRET) {
    if (_secret)
      delete _secret;
    _secret = new dmath::bz;
    if (int result = get_secret(txn, _secret))
      return result;
  }
  if (attrs & ACCT_ATTR_PROMISE) {
    if (_promise)
      bom_os_free(_promise);
    if (int result = get_promise(txn, &_promise))
      return result;
  }
  if (attrs & ACCT_ATTR_VALUES) {
    if (_values)
      delete _values;
    _values = NULL;
    if (int result = get_portfolio(txn, &_values))
      return result;
  }
  if (attrs & ACCT_ATTR_NOTES) {
    if (_notes)
      delete _notes;
    _notes = NULL;
    if (int result = get_note_list(txn, &_notes))
      return result;
  }
  if (attrs & ACCT_ATTR_OFFERS) {
    if (_offers)
      delete _offers;
    _offers = NULL;
    if (int result = get_offer_list(txn, &_offers))
      return result;
  }
  if (attrs & ACCT_ATTR_EVENTS) {
    if (_events)
      delete _events;
    _events = NULL;
    if (int result = get_event_list(txn, &_events))
      return result;
  }
  if (attrs & ACCT_ATTR_FLAGS) {
    if (int result = get_flags(txn, &_flags))
      return result;
  }
  
  return 0;
}


int bom::acct::set(txn_t *txn, u_int32_t attrs) {
  if (attrs & ACCT_ATTR_MODULUS)
    if (int result = set_modulus(txn, *_modulus))
      return result;
  if (attrs & ACCT_ATTR_PUBLIC)
    if (int result = set_public(txn, *_public))
      return result;
  if (attrs & ACCT_ATTR_SECRET)
    if (int result = set_secret(txn, *_secret))
      return result;
  if (attrs & ACCT_ATTR_PROMISE)
    if (int result = set_promise(txn, _promise))
      return result;
  if (attrs & ACCT_ATTR_FLAGS)
    if (int result = set_flags(txn, _flags))
      return result;
  
  return 0;
}

int bom::acct::exists(txn_t *txn, char *exists) {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_ACCT_PUBLIC);

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

int bom::acct::get_modulus(txn_t *txn, dmath::bz *km) const {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_ACCT_MODULUS);

  string_to_dbt(id, &k);
  dbt_init(&v);
  result = db->get(db, txn, &k, &v, 0);
  dbt_clear(&k);

  if (result)
    return result;

  result = dbt_to_bz(v, km);
  dbt_clear(&v);

  if (result)
    return result;
  
  return 0;
}

int bom::acct::set_modulus(txn_t *txn, const dmath::bz& km) const {
  DBT k, v;
  int result;
  DB* db = db_insure_open(DB_ACCT_MODULUS);
  
  string_to_dbt(id, &k);
  bz_to_dbt(km, &v);

  result = db->put(db, txn, &k, &v, 0);
  
  dbt_clear(&k);
  dbt_clear(&v);

  if (result)
    return result;
  
  return 0;
}

int bom::acct::del_modulus(txn_t *txn) const {
  DBT k;
  int result;
  DB *db;
 
  string_to_dbt(id, &k);

  db = db_insure_open(DB_ACCT_MODULUS);
  result = db->del(db, txn, &k, 0);
  dbt_clear(&k);
  if (result == DB_NOTFOUND)
    return 0;
  if (result)
    return result;
  
  return 0;
}

int bom::acct::get_public(txn_t *txn, dmath::bz *kp) const {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_ACCT_PUBLIC);

  string_to_dbt(id, &k);
  dbt_init(&v);

  result = db->get(db, txn, &k, &v, 0);

  dbt_clear(&k);

  if (result)
    return result;

  result = dbt_to_bz(v, kp);
  dbt_clear(&v);

  if (result)
    return result;

  return 0;
}

int bom::acct::del_public(txn_t *txn) const {
  DBT k;
  int result;
  DB *db;
 
  string_to_dbt(id, &k);

  db = db_insure_open(DB_ACCT_PUBLIC);
  result = db->del(db, txn, &k, 0);
  dbt_clear(&k);

  if (result == DB_NOTFOUND)
    return 0;
  if (result)
    return result;
  
  return 0;
}

int bom::acct::set_public(txn_t *txn, const dmath::bz& kp) const {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_ACCT_PUBLIC);

  string_to_dbt(id, &k);
  bz_to_dbt(kp, &v);

  result = db->put(db, txn, &k, &v, 0);
  dbt_clear(&k);
  dbt_clear(&v);

  if (result)
    return result;

  return 0;
}

int bom::acct::get_secret(txn_t *txn, dmath::bz *kp) const {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_ACCT_SECRET);

  string_to_dbt(id, &k);
  dbt_init(&v);

  result = db->get(db, txn, &k, &v, 0);

  dbt_clear(&k);

  if (result)
    return result;

  result = dbt_to_bz(v, kp);
  dbt_clear(&v);

  if (result)
    return result;

  return 0;
}

int bom::acct::del_secret(txn_t *txn) const {
  DBT k;
  int result;
  DB *db;
 
  string_to_dbt(id, &k);

  db = db_insure_open(DB_ACCT_SECRET);
  result = db->del(db, txn, &k, 0);
  dbt_clear(&k);

  if (result == DB_NOTFOUND)
    return 0;
  if (result)
    return result;
  
  return 0;
}

int bom::acct::set_secret(txn_t *txn, const dmath::bz& kp) const {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_ACCT_SECRET);

  string_to_dbt(id, &k);
  bz_to_dbt(kp, &v);

  result = db->put(db, txn, &k, &v, 0);
  dbt_clear(&k);
  dbt_clear(&v);

  if (result)
    return result;

  return 0;
}

int bom::acct::get_value(txn_t *txn, bom::value *val) {
  DBT k, v;
  int result, id_size, denom_size;
  DB *db = db_insure_open(DB_ACCT_VALUE);

  dbt_init(&k);
  k.data = (char *)bom_os_malloc(k.size =
   (id_size = 1 + strlen(id)) +
   (denom_size = 1 + strlen(val->denom.id)));
  memcpy((char *)k.data, id, id_size);
  memcpy((char *)k.data + id_size, val->denom.id, denom_size);
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

int bom::acct::set_value(txn_t *txn, const bom::value &val) {
  DBT k, v;
  int result, id_size, denom_size;
  DB *db = db_insure_open(DB_ACCT_VALUE);

  dbt_init(&k);
  k.data = (char *)bom_os_malloc(k.size =
   (id_size = 1 + strlen(id)) +
   (denom_size = 1 + strlen(val.denom.id)));
  memcpy((char *)k.data, id, id_size);
  memcpy((char *)k.data + id_size, val.denom.id, denom_size);
  bq_to_dbt(val.quant, &v);
  
  result = db->put(db, txn, &k, &v, 0);
  
  dbt_clear(&k);
  dbt_clear(&v);

  if (result)
    return result;

  return 0;
}

int bom::acct::del_value(txn_t *txn, const bom::value &val) {
  DBT k;
  int result, us, ds;
  DB *db = db_insure_open(DB_ACCT_VALUE);

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

int bom::acct::get_value_first(DBC *c, bom::value *val, char *exists) {
  DBT k, v;
  int result;
  
  *exists = 0;

  string_to_dbt(id, &k);
  dbt_init(&v);
  
  result = c->c_get(c, &k, &v, DB_SET_RANGE);

  if (result) {
    dbt_clear(&k);
    return (result == DB_NOTFOUND) ? 0 : result;
  }

  if (strcmp((char *)k.data, id)) {
    dbt_clear(&k);
    dbt_clear(&v);
    return 0;
  }
  
  val->denom.set_id((char *)k.data + strlen((char *)k.data) + 1);
  dbt_clear(&k);
  
  result = dbt_to_bq(v, &val->quant);
  dbt_clear(&v);

  if (result) 
    return result;
  
  *exists = 1;
  return 0;
}

int bom::acct::get_value_next(DBC *c, bom::value *val, char *exists) {
  DBT k, v;
  int result;
  
  *exists = 0;
  
  dbt_init(&k);
  dbt_init(&v);

  if (result = c->c_get(c, &k, &v, DB_NEXT)) {
    dbt_clear(&k);
    return (result == DB_NOTFOUND) ? 0 : result;
  }

  if (strcmp((char *)k.data, id)) {
    dbt_clear(&k);
    dbt_clear(&v);
    return 0;
  }
  
  val->denom.set_id((char *)k.data + strlen((char *)k.data) + 1);
  dbt_clear(&k);

  result = dbt_to_bq(v, &val->quant);
  dbt_clear(&v);

  if (result)
   return result;
  
  *exists = 1;
  return 0;
}

int bom::acct::get_portfolio(txn_t *txn, bom::value **vl) {
  DB *db;
  int result;
  bom::value *v;
  bom::value w;
  DBC* c;
  char exists;

  *vl = NULL;
  db = db_insure_open(DB_ACCT_VALUE);
  result = db->cursor(db, txn, &c, 0);
  if (result) 
    return result;
 
  result = get_value_first(c, &w, &exists);
  if (result) {
    c->c_close(c);
    return result;
  }

  while (exists) {
    v = new bom::value(w);
    v->next = *vl;
    *vl = v;
 
    result = get_value_next(c, &w, &exists);
    if (result) {
      c->c_close(c);
      delete *vl;
      *vl = NULL;
      return result;
    }
  }
  
  if (result = c->c_close(c))
    return result;

  return 0;
}

int bom::acct::set_portfolio(txn_t *txn, bom::value *vl) {
  for (bom::value *v = vl; v; v = v->next) 
    if (int result = set_value(txn, *v))
      return result;
  return 0;
}

int bom::acct::del_portfolio(txn_t *txn) {
  DB *db;
  DBC *c;
  int result;
  char exists;
  bom::value value;

  db = db_insure_open(DB_ACCT_VALUE);

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

int bom::acct::create_challenge(txn_t *txn, dmath::bz* c, char *success) {
  dmath::bz km;
  int result;

  *success = 0;
  if (result = get_modulus(txn, &km))
    return result;

  if (km <= 0) 
    return 0;

  rsa_challenge(c, km);

  *success = 1;
  return 0;
}

int bom::acct::verify_reponse(txn_t *txn,
 const dmath::bz& c0, const dmath::bz& r, char* success) {
  dmath::bz km, kp, c1;
  int result;

  *success = 0;
  if (result = get_modulus(txn, &km))
    return result;
  if (result = get_public(txn, &kp))
    return result;

  if (km <= 0)
    return 0;

  rsa_crypt(&c1, r, kp, km);
  *success = (c0 == c1);
  return 0;
}

int bom::acct::credit(txn_t *txn, const bom::value& v, char* success) {
  bom::value v0;
  int result;
  
  *success = 0;
  v0.set_denom(v.denom);

  if (result = get_value(txn, &v0))
    return result;
  v0.quant += v.quant;
  if (v0.quant == 0) {
    if (result = del_value(txn, v0))
      return result;
  } else if (result = set_value(txn, v0))
    return result;

  {
    bom::event event;
    if (result = event_new_credit(txn, &event, *this, v))
      return result;
  }

  *success = 1;
  return 0;
}

int bom::acct::debit(txn_t *txn, const bom::value& v, char *success) {
  bom::value v0;
  int result;
  bom::acct issuer;
  
  *success = 0;
  v0.set_denom(v.denom);
  
  if (result = get_value(txn, &v0))
    return result;
  v0.quant -= v.quant;

  if (v0.quant < 0) {
    bom::acct_flag flags;

    if (result = get_flags(txn, &flags))
      return result;
    if (!(flags & ACCT_FLAG_SUPER)) {
      if (!(flags & ACCT_FLAG_ISSUER))
	return 0;
      if (result = v0.denom.get_issuer(txn, &issuer))
        return result;
      if (strcmp(issuer.id, id)) {
        if (!(flags & ACCT_FLAG_SUPER))
          return 0;
      }
    }
    if (result = set_value(txn, v0))
      return result;
  } else if (v0.quant == 0) {
    if (result = del_value(txn, v0))
      return result;
  } else if (result = set_value(txn, v0))
    return result;

  {
    bom::event event;
    if (result = event_new_debit(txn, &event, *this, v))
      return result;
  }

  *success = 1;
  return 0;
}

int bom::acct::debit_forced(txn_t *txn, const bom::value& v) {
  bom::value v0;
  int result;
  bom::acct issuer;
  
  v0.set_denom(v.denom);
  
  if (result = get_value(txn, &v0))
    return result;
  v0.quant -= v.quant;

  if (v0.quant == 0) {
    if (result = del_value(txn, v0))
      return result;
  } else if (result = set_value(txn, v0))
    return result;

  {
    bom::event event;
    if (result = event_new_debit(txn, &event, *this, v))
      return result;
  }

  return 0;
}

int bom::acct::get_note_list(txn_t *txn, bom::note **nl) {
  DB *db;
  DBC* c;
  int result;
  bom::note *n;
  DBT k, v;
  
  *nl = NULL;
  
  db = db_insure_open(DB_ACCT_NOTE);
  result = db->cursor(db, txn, &c, 0);
  if (result)
    return result;
  
  string_to_dbt(id, &k);
  dbt_init(&v);

  result = c->c_get(c, &k, &v, DB_SET);
  if (result == DB_NOTFOUND) {
    dbt_clear(&k);
    if (result = c->c_close(c))
      return result;
    return 0;
  }
  if (result) {
    dbt_clear(&k);
    c->c_close(c);
    return result;
  }

  *nl = new bom::note((char *)v.data);

  dbt_clear(&v);
  dbt_init(&v);

  while (!(result = c->c_get(c, &k, &v, DB_NEXT_DUP))) {
    n = new bom::note((char *)v.data);
    n->next = *nl;
    *nl = n;
    dbt_clear(&v);
    dbt_init(&v);
  }

  dbt_clear(&k);
  
  if (result != DB_NOTFOUND) {
    c->c_close(c);
    delete *nl;
    *nl = NULL;
    return result;
  }

  if (result = c->c_close(c)) {
    delete *nl;
    *nl = NULL;
    return result;
  }

  return 0;
}

int bom::acct::del_note_list(txn_t *txn) {
  bom::note *nl;
  int result;

  if (result = get_note_list(txn, &nl))
    return result;
  for (bom::note *n = nl; n; n = n->next)
    if (result = n->destroy(txn)) {
      delete nl;
      return result;
    }

  if (nl)
    delete nl;
  return 0;
}

int bom::acct::get_event_list(txn_t *txn, bom::event **el) {
  DB *db;
  DBC* c;
  int result;
  bom::event *e;
  DBT k, v;
  
  *el = NULL;
  
  db = db_insure_open(DB_ACCT_EVENT);
  result = db->cursor(db, txn, &c, 0);
  if (result)
    return result;
  
  string_to_dbt(id, &k);
  dbt_init(&v);

  result = c->c_get(c, &k, &v, DB_SET);
  if (result == DB_NOTFOUND) {
    dbt_clear(&k);
    if (result = c->c_close(c))
      return result;
    return 0;
  }
  if (result) {
    dbt_clear(&k);
    c->c_close(c);
    return result;
  }

  *el = new bom::event((bom::event_id)strtoul((char *)v.data, NULL, 10));

  dbt_clear(&v);
  dbt_init(&v);

  while (!(result = c->c_get(c, &k, &v, DB_NEXT_DUP))) {
    e = new bom::event((bom::event_id)strtoul((char *)v.data, NULL, 10));
    e->next = *el;
    *el = e;
    dbt_clear(&v);
    dbt_init(&v);
  }

  dbt_clear(&k);
  
  if (result != DB_NOTFOUND) {
    c->c_close(c);
    delete *el;
    *el = NULL;
    return result;
  }

  if (result = c->c_close(c)) {
    delete *el;
    *el = NULL;
    return result;
  }

  return 0;
}

int bom::acct::get_denom_list(txn_t *txn, bom::denom **dl) {
  DB *db;
  DBC *c;
  int result;
  bom::denom *d;
  DBT k, v;
  
  *dl = NULL;
  
  db = db_insure_open(DB_ISSUER_DENOM);
  result = db->cursor(db, txn, &c, 0);
  if (result)
    return result;
  
  string_to_dbt(id, &k);
  dbt_init(&v);

  result = c->c_get(c, &k, &v, DB_SET);
  if (result == DB_NOTFOUND) {
    dbt_clear(&k);
    if (result = c->c_close(c))
      return result;
    return 0;
  }
  if (result) {
    dbt_clear(&k);
    c->c_close(c);
    return result;
  }

  *dl = new bom::denom((char *)v.data);

  dbt_clear(&v);
  dbt_init(&v);

  while (!(result = c->c_get(c, &k, &v, DB_NEXT_DUP))) {
    d = new bom::denom((char *)v.data);
    d->next = *dl;
    *dl = d;
    dbt_clear(&v);
    dbt_init(&v);
  }

  dbt_clear(&k);
  
  if (result != DB_NOTFOUND) {
    c->c_close(c);
    delete *dl;
    *dl = NULL;
    return result;
  }

  if (result = c->c_close(c)) {
    delete *dl;
    *dl = NULL;
    return result;
  }

  return 0;
}

int bom::acct::destroy(txn_t *txn) {
  int result;

  bom::event event;
  if (result = event_new_acct_destroy(txn, &event, *this))
    return result;
 
  if (result = del_note_list(txn))
    return result;
  if (result = del_portfolio(txn))
    return result;
  if (result = del_public(txn))
    return result;
  if (result = del_secret(txn))
    return result;
  if (result = del_modulus(txn))
    return result;
  if (result = del_flags(txn))
    return result;

  return 0;
}

int get_acct_list(txn_t *txn, bom::acct **ul) {
  int result;
  DB *db;
  DBC *c;
  DBT k, v;

  *ul = NULL;

  db = db_insure_open(DB_ACCT_MODULUS);
  result = db->cursor(db, txn, &c, 0);
  if (result)
    return result;

  dbt_init(&k);
  dbt_init(&v);

  while (!(result = c->c_get(c, &k, &v, DB_NEXT))) {
    bom::acct *u = new bom::acct((char *)k.data);
    u->next = *ul;
    *ul = u;

    dbt_clear(&v);
    dbt_clear(&k);
    dbt_init(&k);
    dbt_init(&v);
  }
  
  if (result != DB_NOTFOUND) {
    if (*ul)
      delete *ul;
    *ul = NULL;
    return result;
  }
  
  return 0;
}


int bom::acct::get_flags(txn_t *txn, bom::acct_flag *flags) const {
  DB *db;
  DBT k, v;
  int result;
  
  db = db_insure_open(DB_ACCT_FLAGS);

  string_to_dbt(id, &k);
  dbt_init(&v);

  result = db->get(db, txn, &k, &v, 0);
  dbt_clear(&k);

  if (result) {
    dbt_clear(&v);
    return result;
  }
  
  *(long *)flags = ntohl(*(int *)v.data);
  dbt_clear(&v);
  
  return 0;
}

int bom::acct::set_flags(txn_t *txn, bom::acct_flag flags) const {
  DB *db;
  DBT k, v;
  int result;
  long nflags;
  
  db = db_insure_open(DB_ACCT_FLAGS);

  string_to_dbt(id, &k);
  dbt_init(&v);
  nflags = htonl(flags);
  v.data = (void *)&nflags;
  v.size = sizeof(long);

  result = db->put(db, txn, &k, &v, 0);
  dbt_clear(&k);

  if (result)
    return result;
  
  return 0;
}

int bom::acct::del_flags(txn_t *txn) const {
  DB *db;
  DBT k;
  int result;
  
  db = db_insure_open(DB_ACCT_FLAGS);

  string_to_dbt(id, &k);
  result = db->del(db, txn, &k, 0);
  dbt_clear(&k);

  if (result == DB_NOTFOUND)
    return 0;
  if (result)
    return result;
  
  return 0;
}

int bom::acct::get_promise(txn_t *txn, char **promise) const {
  DB *db;
  DBT k, v;
  int result;
  
  db = db_insure_open(DB_ACCT_PROMISE);

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

int bom::acct::set_promise(txn_t *txn, char *promise) const {
  DB *db;
  DBT k, v;
  int result;
  
  db = db_insure_open(DB_ACCT_PROMISE);

  string_to_dbt(id, &k);
  string_to_dbt(promise, &v);

  result = db->put(db, txn, &k, &v, 0);
  dbt_clear(&k);
  dbt_clear(&v);

  if (result)
    return result;
  
  return 0;
}

int bom::acct::del_promise(txn_t *txn) const {
  DB *db;
  DBT k;
  int result;
  
  db = db_insure_open(DB_ACCT_PROMISE);

  string_to_dbt(id, &k);
  result = db->del(db, txn, &k, 0);
  dbt_clear(&k);

  if (result == DB_NOTFOUND)
    return 0;
  if (result)
    return result;
  
  return 0;
}

int bom::acct::get_offer_list(txn_t *txn, bom::offer **ol) {
  DB *db;
  DBC* c;
  int result;
  bom::offer *o;
  DBT k, v;
  
  *ol = NULL;
  
  db = db_insure_open(DB_ACCT_OFFER);
  result = db->cursor(db, txn, &c, 0);
  if (result)
    return result;
  
  string_to_dbt(id, &k);
  dbt_init(&v);

  result = c->c_get(c, &k, &v, DB_SET);
  if (result == DB_NOTFOUND) {
    dbt_clear(&k);
    c->c_close(c);
    return 0;
  }
  if (result) {
    dbt_clear(&k);
    c->c_close(c);
    return result;
  }

  *ol = new bom::offer((char *)v.data);

  dbt_clear(&v);
  dbt_init(&v);

  while (!(result = c->c_get(c, &k, &v, DB_NEXT_DUP))) {
    o = new bom::offer((char *)v.data);
    o->next = *ol;
    *ol = o;
    dbt_clear(&v);
    dbt_init(&v);
  }

  dbt_clear(&k);
  
  if (result != DB_NOTFOUND) {
    c->c_close(c);
    delete *ol;
    *ol = NULL;
    return result;
  }
  
  if (result = c->c_close(c)) {
    delete *ol;
    *ol = NULL;
    return result;
  }
  
  return 0;
}

int bom::acct::del_offer_list(txn_t *txn) {
  bom::offer *ol;
  int result;

  if (result = get_offer_list(txn, &ol))
    return result;
  for (bom::offer *o = ol; o; o = o->next)
    if (result = o->destroy(txn)) {
      delete ol;
      return result;
    }

  if (ol)
    delete ol;
  return 0;
}

int acct_list_to_string(bom::acct *ol, char **sp) {
  int length = 0, count = 0;

  if (ol) {
    for (bom::acct *o = ol; o; o = o->next)
      length += strlen(o->id) + 1;

    *sp = (char *)bom_os_malloc(length);

    bom::acct *o = ol;
    count += sprintf(*sp + count, "%s", o->id);
    for (o = o->next; o; o = o->next)
      count += sprintf(*sp + count, " %s", o->id);
  } else {
    *sp = (char *)bom_os_malloc(1);
    **sp = '\0';
  }
  
  return 0;
}

int string_to_acct_list(char *_s, bom::acct **olp) {
  char *s, *p, *q;
  int result;
  
  q = s = bom_os_strdup(_s);
  *olp = NULL;

  while (p = strchr(q, ' ')) {
    *p++ = '\0';

    bom::acct *op = new bom::acct;
    op->set_id(q);
    op->next = *olp;
    *olp = op;
    
    while (isspace(*p))
      p++;
    q = p;
  }

  if (*q) {
    bom::acct *op = new bom::acct;
    op->set_id(q);
    op->next = *olp;
    *olp = op;
  }
  
  bom_os_free(s);
  return 0;
}

int acct_flags_to_string(bom::acct_flag f, char **sp) {
  int length;

  length = 0;
  for (int i = 0; *acct_flag_string[i]; i++)
    if (f & (1 << i))
      length += strlen(acct_flag_string[i]) + 1;
  
  if (!(*sp = (char *)bom_os_malloc(length + 3)))
    return -1;
  **sp = '\0';
  length = 0;

  for (int i = 0; *acct_flag_string[i]; i++)
    if (f & (1 << i))
      length += sprintf(*sp + length, "%s ", acct_flag_string[i]);
  if (length > 0)
    *(*sp + --length) = '\0';
  
  return 0;
}

int string_to_acct_flags(char *_s, bom::acct_flag *fp) {
  char *s = bom_os_strdup(_s);
  char *sb = s;
  
  while (isspace(*s))
    s++;

  *fp = 0;
  while (*s) {
    char *p = s;
    while (*p && !isspace(*p))
      p++;
    if (*p) {
      *p++ = '\0';
      while (isspace(*p))
        p++;
    }

    int i = 0;
    for (i = 0; *acct_flag_string[i]; i++) {
      if (!strcmp(s, acct_flag_string[i])) {
	*fp |= (1 << i);
	break;
      }
    }

    if (!*acct_flag_string[i]) {
      bom_os_free(sb);
      return -1;
    }
    s = p;
  }
  
  bom_os_free(sb);
  return 0;
}
