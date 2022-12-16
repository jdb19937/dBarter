// {HEADER}

#define _EVENT_CC
#include "bom.h"

char *event_name[] = {
  "credit",
  "debit",
  "acct_create",
  "acct_destroy",
  "note_create",
  "note_destroy",
  "denom_create",
  "denom_destroy",
  "offer_create",
  "offer_destroy",
  "event_sent",
  "event_received",
  ""
};

bom::event::event() {
  id = 0;
  _type = (bom::event_type)0;
  _data = NULL;
  _accts = NULL;
  next = NULL;
}

bom::event::event(bom::event_id _id) {
  id = _id;
  _type = (bom::event_type)0;
  _data = NULL;
  _accts = NULL;
  next = NULL;
}

bom::event::event(const bom::event& event) {
  id = event.id;
  _type = (bom::event_type)0;
  _data = NULL;
  _accts = NULL;
  next = NULL;
}

bom::event::~event() {
  if (_data)
    delete _data;
  if (_accts)
    delete _accts;
  if (next)
    delete next;
}

int bom::event::get(txn_t *txn, u_int32_t attrs) {
  if (attrs & EVENT_ATTR_TYPE) {
    if (int result = get_type(txn, &_type))
      return result;
  }
  if (attrs & EVENT_ATTR_DATA) {
    if (_data)
      delete _data;
    _data = NULL;
    if (int result = get_data(txn, &_data))
      return result;
  }
  if (attrs & EVENT_ATTR_ACCTS) {
    if (_accts)
      delete _accts;
    _accts = NULL;
    if (int result = get_acct_list(txn, &_accts))
      return result;
  }

  return 0;
}

int bom::event::set(txn_t *txn, u_int32_t attrs) {
  if (attrs & EVENT_ATTR_TYPE)
    if (int result = set_type(txn, _type))
      return result;
  if (attrs & EVENT_ATTR_DATA)
    if (int result = set_data(txn, _data))
      return result;

  return 0;
}

int bom::event::set_id(bom::event_id _id) {
  id = _id;
  return 0;
}

int bom::event::gen_id(txn_t *txn) {
  int result;
  DB *db = db_insure_open(DB_EVENT_TYPE);
  DBT k, v;
  
  int32_to_dbt(0, &k);
  string_to_dbt("0", &v);
  result = db->put(db, txn, &k, &v, DB_APPEND);
  dbt_clear(&v);

  if (result)
    return result;
  
  dbt_to_int32(k, &id);
  
  return 0;
}

int bom::event::exists(txn_t *txn, char *exists) {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_EVENT_TYPE);

  *exists = 0;

  int32_to_dbt((u_int32_t)id, &k);
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

int bom::event::get_type(txn_t *txn, bom::event_type *type) {
  DBT k, v;
  int result;
  int i;
  DB *db = db_insure_open(DB_EVENT_TYPE);

  int32_to_dbt((u_int32_t)id, &k);
  dbt_init(&v);

  result = db->get(db, txn, &k, &v, 0);
  dbt_clear(&k);

  if (result)
    return result;
  
  for (i = 0; *event_name[i]; i++)
    if (!strcmp(event_name[i], (char *)v.data)) {
      *type = i;
      break;
    }

  dbt_clear(&v);
  if (!*event_name[i])
    return EINVAL;

  return 0;
}

int bom::event::set_type(txn_t *txn, bom::event_type type) {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_EVENT_TYPE);

  int32_to_dbt((u_int32_t)id, &k);
  string_to_dbt(event_name[type], &v);

  result = db->put(db, txn, &k, &v, 0);
  dbt_clear(&k);
  dbt_clear(&v);

  if (result)
    return result;
  
  return 0;
}

int bom::event::del_type(txn_t *txn) {
  DB *db;
  DBT k;
  int result;
  
  int32_to_dbt((u_int32_t)id, &k);

  db = db_insure_open(DB_EVENT_TYPE);
  result = db->del(db, txn, &k, 0);
  dbt_clear(&k);
  if (result == DB_NOTFOUND) 
    return 0;
  if (result)
    return result;
  
  return 0;
}

int bom::event::get_data(txn_t *txn, char **data) {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_EVENT_DATA);

  int32_to_dbt((u_int32_t)id, &k);
  dbt_init(&v);

  result = db->get(db, txn, &k, &v, 0);
  dbt_clear(&k);

  if (result)
    return result;

  *data = (char *)v.data;
  return 0;
}

int bom::event::set_data(txn_t *txn, char *data) {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_EVENT_DATA);

  int32_to_dbt((u_int32_t)id, &k);
  string_to_dbt(data, &v);

  result = db->put(db, txn, &k, &v, 0);

  dbt_clear(&k);
  dbt_clear(&v);

  if (result)
    return result;
  
  return 0;
}

int bom::event::del_data(txn_t *txn) {
  DB *db;
  DBT k;
  int result;
  
  int32_to_dbt((u_int32_t)id, &k);

  db = db_insure_open(DB_EVENT_DATA);
  result = db->del(db, txn, &k, 0);
  dbt_clear(&k);
  if (result == DB_NOTFOUND) 
    return 0;
  if (result)
    return result;
  
  return 0;
}

int bom::event::get_acct_list(txn_t *txn, bom::acct **ul) const {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_EVENT_ACCT);

  *ul = NULL;

  int32_to_dbt((u_int32_t)id, &k);
  dbt_init(&v);

  result = db->get(db, txn, &k, &v, 0);
  dbt_clear(&k);

  if (result)
    return result;

  bom::acct *last = NULL;
  for (int i = 0; i < v.size; i++) {
    bom::acct *u = new bom::acct((char *)v.data + i);
    if (last) {
      last->next = u;
      last = u;
    } else
      last = *ul = u;
    i += strlen((char *)v.data);
  }
    
  dbt_clear(&v);

  return 0;
}

int bom::event::set_acct_list(txn_t *txn, const bom::acct* ul) const {
  DBT k, v;
  int result;
  DB *db = db_insure_open(DB_EVENT_ACCT);
  DB *dbr = db_insure_open(DB_ACCT_EVENT);
  DBC *c;

  int32_to_dbt((u_int32_t)id, &k);
  
  {
    dbt_init(&v);
    for (const bom::acct *u = ul; u; u = u->next)
      v.size += strlen(u->id) + 1;
    v.data = bom_os_malloc(v.size);

    int k = 0;
    for (const bom::acct *u = ul; u; u = u->next) {
      strcpy((char *)v.data + k, u->id);
      k += strlen(u->id) + 1;
    }
  }

  result = db->put(db, txn, &k, &v, 0);
  dbt_clear(&k);
  dbt_clear(&v);

  if (result)
    return result;
  
  result = dbr->cursor(dbr, txn, &c, 0);
  if (result)
    return result;

  for (const bom::acct *u = ul; u; u = u->next) {
    string_to_dbt(u->id, &k);
    {
      char seventid[256];
      sprintf(seventid, "%d", id);
      string_to_dbt(seventid, &v);
    }
  
    result = c->c_put(c, &k, &v, DB_KEYLAST);

    dbt_clear(&k);
    dbt_clear(&v);

    if (result) {
      c->c_close(c);
      return result;
    }
  }
  
  if (result = c->c_close(c))
    return result;
  
  return 0;
}

int bom::event::del_acct_list(txn_t *txn) const {
  DBT k, v;
  DB *db;
  DBC *c;
  int result;

  int32_to_dbt((u_int32_t)id, &k);
  dbt_init(&v);
  
  db = db_insure_open(DB_EVENT_ACCT);

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

  db = db_insure_open(DB_ACCT_EVENT);
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

int bom::event::destroy(txn_t *txn) {
  if (int result = del_type(txn))
    return result;
  if (int result = del_data(txn))
    return result;
  if (int result = del_acct_list(txn))
    return result;
  return 0;
}

int get_last_event(txn_t *txn, bom::event *event) {
  DB *db = db_insure_open(DB_EVENT_TYPE);
  DBT k, v;
  DBC *c;
  int result;
  
  dbt_init(&k);
  dbt_init(&v);

  if (result = db->cursor(db, txn, &c, 0))
    return result;
  result = c->c_get(c, &k, &v, DB_LAST);
  if (result) {
    c->c_close(c);
    return result;
  }

  if (result = c->c_close(c))
    return result;
  
  dbt_to_int32(k, &event->id);
  dbt_clear(&k);
  dbt_clear(&v);
  
  return 0;
}

int event_new_credit(txn_t *txn, bom::event *event,
 const bom::acct &acct, const bom::value &value) {
  char *svalue;
  char *data;
  
  value_to_string(value, &svalue);
  sprintf_alloc(&data, "acct %s\nvalue %s", acct.id, svalue);
  bom_os_free(svalue);
  
  if (int result = event->gen_id(txn)) {
    bom_os_free(data);
    return result;
  }
  if (int result = event->set_data(txn, data)) {
    bom_os_free(data);
    return result;
  }
  bom_os_free(data);

  if (int result = event->set_type(txn, EVENT_TYPE_CREDIT))
    return result;
  if (int result = event->set_acct_list(txn, &acct))
    return result;
  
  return 0;
}


int event_new_debit(txn_t *txn, bom::event *event,
 const bom::acct &acct, const bom::value &value) {
  char *svalue;
  char *data;
  
  value_to_string(value, &svalue);
  sprintf_alloc(&data, "acct %s\nvalue %s", acct.id, svalue);
  bom_os_free(svalue);
  
  if (int result = event->gen_id(txn)) {
    bom_os_free(data);
    return result;
  }
  if (int result = event->set_data(txn, data)) {
    bom_os_free(data);
    return result;
  }
  bom_os_free(data);

  if (int result = event->set_type(txn, EVENT_TYPE_DEBIT))
    return result;
  if (int result = event->set_acct_list(txn, &acct))
    return result;
  
  return 0;
}

int event_new_note_create(txn_t *txn, bom::event *event, const bom::note &note) {
  char *data;
  
  if (int result = event->gen_id(txn)) 
    return result;
  if (int result = event->set_acct_list(txn, note._issuer))
    return result;

  sprintf_alloc(&data, "note %s", note.id);
  if (int result = event->set_data(txn, data)) {
    bom_os_free(data);
    return result;
  }

  bom_os_free(data);

  if (int result = event->set_type(txn, EVENT_TYPE_NOTE_CREATE)) 
    return result;
  
  return 0;
}

int event_new_note_destroy(txn_t *txn, bom::event *event, const bom::note &note) {
  char *data;
  
  if (int result = event->gen_id(txn)) 
    return result;
  if (int result = event->set_acct_list(txn, note._issuer))
    return result;

  sprintf_alloc(&data, "note %s", note.id);
  if (int result = event->set_data(txn, data)) {
    bom_os_free(data);
    return result;
  }

  bom_os_free(data);

  if (int result = event->set_type(txn, EVENT_TYPE_NOTE_DESTROY)) 
    return result;
  
  return 0;
}

int event_new_acct_create(txn_t *txn, bom::event *event, const bom::acct &acct) {
  char *data;
  
  if (int result = event->gen_id(txn)) 
    return result;
  if (int result = event->set_acct_list(txn, &acct))
    return result;

  sprintf_alloc(&data, "acct %s", acct.id);
  if (int result = event->set_data(txn, data)) {
    bom_os_free(data);
    return result;
  }

  bom_os_free(data);

  if (int result = event->set_type(txn, EVENT_TYPE_ACCT_CREATE)) 
    return result;
  
  return 0;
}

int event_new_acct_destroy(txn_t *txn, bom::event *event, const bom::acct &acct) {
  char *data;
  
  if (int result = event->gen_id(txn)) 
    return result;
  if (int result = event->set_acct_list(txn, &acct))
    return result;

  sprintf_alloc(&data, "acct %s", acct.id);
  if (int result = event->set_data(txn, data)) {
    bom_os_free(data);
    return result;
  }

  bom_os_free(data);

  if (int result = event->set_type(txn, EVENT_TYPE_ACCT_DESTROY)) 
    return result;
  
  return 0;
}

int event_new_denom_create(txn_t *txn, bom::event *event, const bom::denom &denom) {
  char *data;
  
  if (int result = event->gen_id(txn)) 
    return result;
  if (int result = event->set_acct_list(txn, denom._issuer))
    return result;

  sprintf_alloc(&data, "denom %s", denom.id);
  if (int result = event->set_data(txn, data)) {
    bom_os_free(data);
    return result;
  }

  bom_os_free(data);

  if (int result = event->set_type(txn, EVENT_TYPE_DENOM_CREATE)) 
    return result;
  
  return 0;
}

int event_new_denom_destroy(txn_t *txn, bom::event *event, const bom::denom &denom) {
  char *data;
  
  if (int result = event->gen_id(txn)) 
    return result;
  if (int result = event->set_acct_list(txn, denom._issuer))
    return result;

  sprintf_alloc(&data, "denom %s", denom.id);
  if (int result = event->set_data(txn, data)) {
    bom_os_free(data);
    return result;
  }

  bom_os_free(data);

  if (int result = event->set_type(txn, EVENT_TYPE_DENOM_DESTROY)) 
    return result;
  
  return 0;
}

int event_new_offer_create(txn_t *txn, bom::event *event, const bom::offer &offer) {
  char *data;
  
  if (int result = event->gen_id(txn)) 
    return result;
  if (int result = event->set_acct_list(txn, offer.issuer))
    return result;

  sprintf_alloc(&data, "offer %s", offer.id);
  if (int result = event->set_data(txn, data)) {
    bom_os_free(data);
    return result;
  }

  bom_os_free(data);

  if (int result = event->set_type(txn, EVENT_TYPE_OFFER_CREATE)) 
    return result;
  
  return 0;
}

int event_new_offer_destroy(txn_t *txn, bom::event *event, const bom::offer &offer) {
  char *data;
  
  if (int result = event->gen_id(txn)) 
    return result;
  if (int result = event->set_acct_list(txn, offer.issuer))
    return result;

  sprintf_alloc(&data, "offer %s", offer.id);
  if (int result = event->set_data(txn, data)) {
    bom_os_free(data);
    return result;
  }

  bom_os_free(data);

  if (int result = event->set_type(txn, EVENT_TYPE_OFFER_DESTROY)) 
    return result;
  
  return 0;
}

int event_new_event_received(txn_t *txn,
 bom::event *event, const bom::acct &from,
 const bom::acct &to, char *body) {
  char *data;
  
  if (int result = event->gen_id(txn)) 
    return result;
  if (int result = event->set_acct_list(txn, &to))
    return result;

  sprintf_alloc(&data, "from %s\nto %s\ndata %s", from.id, to.id, body);
  if (int result = event->set_data(txn, data)) {
    bom_os_free(data);
    return result;
  }

  bom_os_free(data);

  if (int result = event->set_type(txn, EVENT_TYPE_EVENT_RECEIVED)) 
    return result;
  
  return 0;
}

int event_new_event_sent(txn_t *txn,
 bom::event *event, const bom::acct &from,
 const bom::acct &to, char *body) {
  char *data;
  
  if (int result = event->gen_id(txn)) 
    return result;
  if (int result = event->set_acct_list(txn, &from))
    return result;

  sprintf_alloc(&data, "from %s\nto %s\ndata %s", from.id, to.id, body);
  if (int result = event->set_data(txn, data)) {
    bom_os_free(data);
    return result;
  }

  bom_os_free(data);

  if (int result = event->set_type(txn, EVENT_TYPE_EVENT_SENT)) 
    return result;
  
  return 0;
}

int event_to_string(const bom::event &event, char **s) {
  if (!(*s = (char *)bom_os_malloc(32)))
    return -1;
  sprintf(*s, "event:%d", event.id);
  return 0;
}

int string_to_event(char *s, bom::event *event) {
  bom::event_id event_id;

  if (!strncmp(s, "event:", 6))
    s += 6;
  if ((event_id = (bom::event_id)strtoul(s, NULL, 10)) < 1)
    return -1;
  event->set_id(event_id);
  return 0;
}

int event_list_to_string(bom::event *ol, char **sp) {
  int length = 0, count = 0;

  if (ol) {
    for (bom::event *o = ol; o; o = o->next)
      length += (int)log10((double)o->id) + 2;

    *sp = (char *)bom_os_malloc(length);

    bom::event *o = ol;
    count += sprintf(*sp + count, "%d", o->id);
    for (o = o->next; o; o = o->next)
      count += sprintf(*sp + count, " %d", o->id);
  } else {
    *sp = (char *)bom_os_malloc(1);
    **sp = '\0';
  }
  
  return 0;
}

int string_to_event_list(char *_s, bom::event **olp) {
  char *s, *p, *q;
  int result;
  
  q = s = bom_os_strdup(_s);
  *olp = NULL;

  while (p = strchr(q, ' ')) {
    *p++ = '\0';

    bom::event *op = new bom::event;
    string_to_event(q, op);
    op->next = *olp;
    *olp = op;
    
    while (isspace(*p))
      p++;
    q = p;
  }

  if (*q) {
    bom::event *op = new bom::event;
    string_to_event(q, op);
    op->next = *olp;
    *olp = op;
  }
  
  bom_os_free(s);
  return 0;
}
