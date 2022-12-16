// {HEADER}

#define _OFFER_CC
#include "bom.h"

bom::offer::offer() : bom::object() {
  issuer = NULL;
  bid_value = NULL;
  ask_value = NULL;
  next = NULL;
}

bom::offer::offer(char *_id) : bom::object(_id) {
  issuer = NULL;
  bid_value = NULL;
  ask_value = NULL;
  next = NULL;
}

bom::offer::offer(const bom::offer& offer) : bom::object(offer) {
  issuer = NULL;
  bid_value = NULL;
  ask_value = NULL;
  next = NULL;
}

bom::offer::~offer() {
  if (issuer)
    delete issuer;
  if (bid_value)
    delete bid_value;
  if (ask_value)
    delete ask_value;
  if (next)
    delete next;
}

int bom::offer::get(txn_t *txn, u_int32_t attrs) {
  if (attrs & OFFER_ATTR_ISSUER) {
    if (issuer)
      delete issuer;
    issuer = new bom::acct;
    if (int result = get_issuer(txn, issuer))
      return result;
  }
  if (attrs & OFFER_ATTR_BID_VALUE) {
    if (bid_value)
      delete bid_value;
    bid_value = new bom::value;
    if (int result = get_bid_value(txn, bid_value))
      return result;
  }
  if (attrs & OFFER_ATTR_ASK_VALUE) {
    if (ask_value)
      delete ask_value;
    ask_value = new bom::value;
    if (int result = get_ask_value(txn, ask_value))
      return result;
  }
  
  return 0;
}

int bom::offer::set(txn_t *txn, u_int32_t attrs) {
  if (attrs & OFFER_ATTR_ISSUER)
    if (int result = set_issuer(txn, *issuer))
      return result;
  if (attrs & OFFER_ATTR_BID_VALUE)
    if (int result = set_bid_value(txn, *bid_value))
      return result;
  if (attrs & OFFER_ATTR_ASK_VALUE)
    if (int result = set_ask_value(txn, *ask_value))
      return result;
  
  return 0;
}

int bom::offer::gen_id() {
  if (id)
    bom_os_free(id);
  if (!(id = (char *)bom_os_malloc(5 + 1 + 8 + 1 + 8 + 1))) 
    return -1;
  sprintf(id, "offer:%08X%08X",
   (unsigned int)time(0), (unsigned int)rand());
  return 0;
}

int bom::offer::exists(txn_t *txn, char *exists) {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_OFFER_ISSUER);

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


int bom::offer::get_issuer(txn_t *txn, bom::acct *_issuer) {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_OFFER_ISSUER);

  string_to_dbt(id, &k);
  dbt_init(&v);

  result = db->get(db, txn, &k, &v, 0);
  dbt_clear(&k);

  if (result)
    return result;

  _issuer->set_id((char *)v.data);
  dbt_clear(&v);
  return 0;
}


int bom::offer::set_issuer(txn_t *txn, const bom::acct& _issuer) {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_OFFER_ISSUER);
  DB *dbr = db_insure_open(DB_ACCT_OFFER);
  DBC *c;

  string_to_dbt(id, &k);
  string_to_dbt(_issuer.id, &v);
  result = db->put(db, txn, &k, &v, 0);
  dbt_clear(&k);
  dbt_clear(&v);

  if (result)
    return result;
  
  result = dbr->cursor(dbr, txn, &c, 0);
  if (result)
    return result;

  string_to_dbt(_issuer.id, &k);
  string_to_dbt(id, &v);
  
  result = c->c_get(c, &k, &v, DB_GET_BOTH);
  
  if (result == 0) {
    dbt_clear(&k);
    dbt_clear(&v);
    c->c_close(c);
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
  c->c_close(c);

  if (result)
    return result;
  
  return 0;
}

int bom::offer::del_issuer(txn_t *txn) {
  DBT k, v;
  DB *db;
  DBC *c;
  int result;

  string_to_dbt(id, &k);
  dbt_init(&v);
  
  db = db_insure_open(DB_OFFER_ISSUER);

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

  db = db_insure_open(DB_ACCT_OFFER);
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
  if (result == DB_NOTFOUND) {
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

int bom::offer::get_bid_value(txn_t *txn, bom::value *_bid_value) {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_OFFER_BID_VALUE);

  string_to_dbt(id, &k);
  dbt_init(&v);
  result = db->get(db, txn, &k, &v, 0);
  dbt_clear(&k);

  if (result)
    return result;
  
  string_to_value((char *)v.data, _bid_value);
  return 0;
}

int bom::offer::set_bid_value(txn_t *txn, const bom::value &_bid_value) {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_OFFER_BID_VALUE);

  string_to_dbt(id, &k);
  dbt_init(&v);
  {
    char *svalue;
    value_to_string(_bid_value, &svalue);
    string_to_dbt(svalue, &v);
    bom_os_free(svalue);
  }

  result = db->put(db, txn, &k, &v, 0);
  dbt_clear(&k);
  dbt_clear(&v);

  if (result)
    return result;
  
  return 0;
}

int bom::offer::del_bid_value(txn_t *txn) {
  DBT k;
  int result;
  DB *db = db_insure_open(DB_OFFER_BID_VALUE);

  string_to_dbt(id, &k);
  result = db->del(db, txn, &k,  0);
  dbt_clear(&k);

  if (result == DB_NOTFOUND)
    return 0;
  if (result)
    return result;
  
  return 0;
}



int bom::offer::get_ask_value(txn_t *txn, bom::value *_ask_value) {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_OFFER_ASK_VALUE);

  string_to_dbt(id, &k);
  dbt_init(&v);
  result = db->get(db, txn, &k, &v, 0);
  dbt_clear(&k);

  if (result)
    return result;
  
  string_to_value((char *)v.data, _ask_value);
  return 0;
}

int bom::offer::set_ask_value(txn_t *txn, const bom::value &_ask_value) {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_OFFER_ASK_VALUE);

  string_to_dbt(id, &k);
  dbt_init(&v);
  {
    char *svalue;
    value_to_string(_ask_value, &svalue);
    string_to_dbt(svalue, &v);
    bom_os_free(svalue);
  }

  result = db->put(db, txn, &k, &v, 0);
  dbt_clear(&k);
  dbt_clear(&v);

  if (result)
    return result;
  
  return 0;
}

int bom::offer::del_ask_value(txn_t *txn) {
  DBT k;
  int result;
  DB *db = db_insure_open(DB_OFFER_ASK_VALUE);

  string_to_dbt(id, &k);
  result = db->del(db, txn, &k,  0);
  dbt_clear(&k);

  if (result == DB_NOTFOUND)
    return 0;
  if (result)
    return result;
  
  return 0;
}

int bom::offer::set_index(txn_t *txn,
 const bom::denom& ask, const bom::denom& bid, const dmath::bq &ratio) {
  DB *db;
  DBT k, v;
  int result;

  db = db_insure_open(DB_OFFER_INDEX);
  {
    int ads, bds;
    dbt_init(&k);
    k.size =
     (ads = strlen(ask.id) + 1) +
     (bds = strlen(bid.id) + 1);
    k.data = bom_os_malloc(k.size);
    memcpy((char *)k.data, ask.id, ads);
    memcpy((char *)k.data + ads, bid.id, bds);
  }
  
  {
    int ids, qs;
    char *sratio;
    bq_to_string(ratio, &sratio);

    dbt_init(&v);
    v.size =
      (ids = strlen(id) + 1) +
      (qs = strlen(sratio) + 1);
    v.data = bom_os_malloc(v.size);
    memcpy((char *)v.data, id, ids);
    memcpy((char *)v.data + ids, sratio, qs);
    bom_os_free(sratio);
  }

  result = db->put(db, txn, &k, &v, 0);
  dbt_clear(&k);
  dbt_clear(&v);
  if (result)
    return result;

  return 0;
}

int bom::offer::set_index(txn_t *txn, const bom::value& ask, const bom::value &bid) {
  dmath::bq ratio;
  
  if (bid.quant == 0)
    return -1;
  ratio = ask.quant;
  ratio /= bid.quant;
  
  return set_index(txn, ask.denom, bid.denom, ratio);
}

int bom::offer::del_index(txn_t *txn, const bom::denom &ask, const bom::denom &bid) {
  DB *db;
  DBC *c;
  DBT k, v;
  int result;

  db = db_insure_open(DB_OFFER_INDEX);

  if (result = db->cursor(db, txn, &c, 0))
    return result;

  {
    int ads, bds;
    dbt_init(&k);
    k.size = (ads = strlen(ask.id) + 1) + (bds = strlen(bid.id) + 1);
    k.data = bom_os_malloc(k.size);
    memcpy((char *)k.data, ask.id, ads);
    memcpy((char *)k.data + ads, bid.id, bds);
  }
  
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

  while (strcmp((char *)v.data, id)) {
    dbt_clear(&v);
    dbt_init(&v);

    result = c->c_get(c, &k, &v, DB_NEXT_DUP);
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
  }

  dbt_clear(&k);
  dbt_clear(&v);
  
  if (result = c->c_del(c, 0)) {
    c->c_close(c);
    return result;
  }
  
  if (result = c->c_close(c))
    return result;

  return 0;
}

int bom::offer::destroy(txn_t *txn) {
  int result;
  bom::value ask, bid;

  bom::event event;
  if (result = event_new_offer_destroy(txn, &event, *this))
    return result;
 
  result = get_ask_value(txn, &ask);
  if (result == DB_NOTFOUND)
    return 0;
  if (result)
    return result;

  result = get_bid_value(txn, &bid);
  if (result == DB_NOTFOUND)
    return 0;
  if (result)
    return result;

  if (result = del_bid_value(txn))
    return result;
  if (result = del_ask_value(txn))
    return result;
  if (result = del_issuer(txn))
    return result;
  if (result = del_index(txn, ask.denom, bid.denom))
    return result;

  return 0;
}

int find_best_offer(txn_t *txn,
 const bom::denom &ask, const bom::denom &bid,
 bom::offer *offer, dmath::bq *ratio, char *exists) {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_OFFER_INDEX);
  DBC *c;
  
  *exists = 0;
  result = db->cursor(db, txn, &c, 0);
  if (result)
    return result;
  
  {
    int ads, bds;
    dbt_init(&k);
    k.size = (ads = strlen(ask.id) + 1) + (bds = strlen(bid.id) + 1);
    k.data = bom_os_malloc(k.size);
    memcpy((char *)k.data, ask.id, ads);
    memcpy((char *)k.data + ads, bid.id, bds);
  }
  
  dbt_init(&v);
  
  result = c->c_get(c, &k, &v, DB_SET);
  if (result == DB_NOTFOUND) {
    dbt_clear(&k);
    c->c_close(c);
    return 0;
  } else if (result) {
    dbt_clear(&k);
    c->c_close(c);
    return result;
  }

  offer->set_id((char *)v.data);
  {
    char *sratio;
    sratio = (char *)v.data + strlen((char *)v.data) + 1;
    string_to_bq(sratio, ratio);
  }

  dbt_clear(&k);
  dbt_clear(&v);

  if (result = c->c_close(c))
    return result;
  
  *exists = 1;
  return 0;
}

int find_all_offers(txn_t *txn,
 const bom::denom &ask, const bom::denom &bid,
 bom::offer **ol) {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_OFFER_INDEX);
  DBC *c;
  
  *ol = NULL;
  result = db->cursor(db, txn, &c, 0);
  if (result)
    return result;
  
  {
    int ads, bds;
    dbt_init(&k);
    k.size = (ads = strlen(ask.id) + 1) + (bds = strlen(bid.id) + 1);
    k.data = bom_os_malloc(k.size);
    memcpy((char *)k.data, ask.id, ads);
    memcpy((char *)k.data + ads, bid.id, bds);
  }
  
  dbt_init(&v);
  
  result = c->c_get(c, &k, &v, DB_SET);
  if (result == DB_NOTFOUND) {
    dbt_clear(&k);
    c->c_close(c);
    return 0;
  } else if (result) {
    dbt_clear(&k);
    c->c_close(c);
    return result;
  }

  while (1) {
    bom::offer *o = new bom::offer;
    o->set_id((char *)v.data);
    o->next = *ol;
    *ol = o;

    dbt_clear(&v);
    dbt_init(&v);
    result = c->c_get(c, &k, &v, DB_NEXT_DUP);
    if (result == DB_NOTFOUND) {
      break;
    } else if (result) {
      dbt_clear(&k);
      c->c_close(c);
      if (*ol)
	delete *ol;
      return result;
    }
  }

  dbt_clear(&k);
  if (result = c->c_close(c)) {
    if (*ol)
      delete *ol;
    return result; 
  }

  return 0;
}

int offer_list_to_string(bom::offer *ol, char **sp) {
  int length = 0, count = 0;

  if (ol) {
    for (bom::offer *o = ol; o; o = o->next)
      length += strlen(o->id) + 1;

    *sp = (char *)bom_os_malloc(length);

    bom::offer *o = ol;
    count += sprintf(*sp + count, "%s", o->id);
    for (o = o->next; o; o = o->next)
      count += sprintf(*sp + count, " %s", o->id);
  } else {
    *sp = (char *)bom_os_malloc(1);
    **sp = '\0';
  }
  
  return 0;
}

int string_to_offer_list(char *_s, bom::offer **olp) {
  char *s, *p, *q;
  int result;
  
  q = s = bom_os_strdup(_s);
  *olp = NULL;

  while (p = strchr(q, ' ')) {
    *p++ = '\0';

    bom::offer *op = new bom::offer;
    op->set_id(q);
    op->next = *olp;
    *olp = op;
    
    while (isspace(*p))
      p++;
    q = p;
  }

  if (*q) {
    bom::offer *op = new bom::offer;
    op->set_id(q);
    op->next = *olp;
    *olp = op;
  }
  
  bom_os_free(s);
  return 0;
}
