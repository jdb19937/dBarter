// {HEADER}

#define _SESSION_CC
#include "bif.h"

bif::session *session_list = NULL;

bif::session::session(bif::shell *_shell) : bom::object() {
  shell = _shell;
  gen_id();
  t0 = time(0);

  prev = NULL;
  next = NULL;
  link();
}

bif::session::~session() {
  unlink();
}

bif::session *find_session(char *id) {
  for (bif::session *s = session_list; s; s = s->next)
    if (s->id && !strcmp(id, s->id))
      return s;
  return NULL;
}

void expire_sessions() {
  time_t t1 = time(0);
  time_t dt = 60 * 60 * 4;
  bif::session *s_next;

  for (bif::session *s = session_list; s; s = s_next) {
    s_next = s->next;
    if (t1 - s->t0 > dt)
      delete s;
  }
}

void bif::session::link() {
  next = session_list;
  session_list = this;
  if (session_list)
    session_list->prev = this;
  prev = NULL;
}

void bif::session::unlink() {
  if (prev)
    prev->next = next;
  if (next)
    next->prev = prev;
  if (session_list == this)
    session_list = next;
}

int bif::session::gen_id() {
  if (id)
    bom_os_free(id);
  if (!(id = (char *)bom_os_malloc(7 + 1 + 8 + 8 + 1))) 
    return -1;
  sprintf(id, "SESSION:%08X%08X",
   (unsigned int)time(0), (unsigned int)rand());
  return 0;
}

