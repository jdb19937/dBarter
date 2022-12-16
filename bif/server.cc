// {HEADER}

#define _S_SERVER_CC
#include "bom.h"
#include "bif.h"

bif::server::server() {
  client_list = NULL;
  market = new bif::market(this);
  event_id = 0;
  process_events();
}

bif::server::~server() {
  delete market;
  while (bif::shell *c = client_list)
    delete c;
  event_thread.cancel();
}

int bif::server::open(char *host, int port) {
  int on = 1, result;

  if ((s = ::socket(AF_INET, SOCK_STREAM, SOL_TCP)) < 0)
    return s;
  
  if ((result = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0)
    return result;

  if ((result = find_host(host, port, &addr)) < 0)
    return result;

  if ((result = bind(s, (struct sockaddr *)&addr, sizeof(addr))) < 0)
    return result;

  if ((result = listen(s, 16)) < 0)
    return result;
  
  site.set_host(host);
  site.set_port(port);
  
  return 0;
}

int bif::server::open(int port) {
  return open("", port);
}

int bif::server::open(const bif::site &site) {
  return open(site.host, site.port);
}

int bif::server::accept() {
  int t;
  struct sockaddr_in from;
  unsigned int size = sizeof(struct sockaddr_in);
  bif::shell *n;

  if ((t = ::accept(s, (struct sockaddr *)&from, &size)) < 0)
    return t;

  n = new bif::shell(t, &from, this);
  return 0;
}


void bif::server::lock_acct(const bom::acct &acct) {
  lock();
  
  for (bif::shell *c = client_list; c; c = c->next) {
    if (!c->acct.id)
      continue;
    if (strcmp(c->acct.id, acct.id))
      continue;
    c->lock();
  }
  
  unlock();
}

void bif::server::unlock_acct(const bom::acct &acct) {
  lock();
  
  for (bif::shell *c = client_list; c; c = c->next) {
    if (!c->acct.id)
      continue;
    if (strcmp(c->acct.id, acct.id))
      continue;
    c->unlock();
  }
  
  unlock();
}

void bif::server::write_acct(const bom::acct &acct, const bif::message &m) {
  lock();
  
  for (bif::shell *c = client_list; c; c = c->next) {
    if (!c->acct.id)
      continue;
    if (strcmp(c->acct.id, acct.id))
      continue;
    if (c->session)
      continue;
    c->writef("%03d %s\n%s\n.\n", m.code, m.desc, m.data);
  }
  
  unlock();
}

void bif::server::writef_acct(const bom::acct &acct, char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  lock();
  
  for (bif::shell *c = client_list; c; c = c->next) {
    if (!c->acct.id)
      continue;
    if (strcmp(c->acct.id, acct.id))
      continue;
    if (c->session)
      continue;
    vdprintf(c->s, fmt, ap);
  }

  unlock();
  va_end(ap);
}

void bif::server::event_update() {
  event_mutex.lock();
  event_condition.signal();
  event_mutex.unlock();
}

void *_process_events(void *_sv) {
  txn_t *txn;
  bif::server *sv = (bif::server *)_sv;
  int result;
  
  while (1) {
    bom::event event;
    bom::event_type type;
    bom::acct *ul;
    char *data;

    sv->event_mutex.lock();
    sv->event_condition.wait(&sv->event_mutex);
    sv->event_mutex.unlock();

  again:
    result = txn_new(&txn);
    if (result) 
      errorf("database error: %d\n.\n", result);
    
    if (sv->event_id == 0) {
      get_last_event(txn, &event);
      sv->event_id = ++event.id;
    } else
      event.set_id(sv->event_id);
    
    result = event.get_type(txn, &type);
    if (result == EAGAIN) {
      txn_abort(txn);
      goto again;
    } else if (result == DB_NOTFOUND) {
      txn_abort(txn);
      continue;
    } else if (result)
      errorf("database error: %d\n.\n", result);

    result = event.get_data(txn, &data);
    if (result == EAGAIN) {
      txn_abort(txn);
      goto again;
    } else if (result == DB_NOTFOUND) {
      txn_abort(txn);
      continue;
    } else if (result)
      errorf("database error: %d\n.\n", result);

    result = event.get_acct_list(txn, &ul);
    if (result == EAGAIN) {
      txn_abort(txn);
      goto again;
    } else if (result == DB_NOTFOUND) {
      txn_abort(txn);
      continue;
    } else if (result)
      errorf("database error: %d\n.\n", result);
    
    result = txn_commit(txn);
    if (result == EAGAIN) {
      txn_abort(txn);
      goto again;
    } else if (result)
      errorf("database error: %d\n.\n", result);

    sv->event_id++;
    
    for (bom::acct *u = ul; u; u = u->next) {
      for (bif::shell *s = sv->client_list; s; s = s->next) {
	if (!s->status || strcmp(s->acct.id, u->id))
	  goto again;
	s->lock();
        s->writef("%03d new event (%s)\nid %d\ntype %s\n",
         CODE_EVENT, event_name[type], event.id, event_name[type]);
        s->writef("%s\n.\n", data);
        s->unlock();
      }
    }
    
  }
  
  return NULL;
}

void bif::server::process_events() {
  event_thread.spawn(_process_events, this);
  event_thread.detach();
}
