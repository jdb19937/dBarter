// {HEADER}

#include <errno.h>

#define _MUTEX_RW_CC
#include "dthread.h"

dthread::mutex_rw::mutex_rw(...) {
  readers = 0;
  writers = 0;
}
  
dthread::mutex_rw::~mutex_rw() {

}

int dthread::mutex_rw::rlock() {
  m.lock();
  while (writers)
    c.wait(&m);
  readers++;
  m.unlock();
  return 0;
}

int dthread::mutex_rw::rlock_nb() {
  m.lock();

  if (writers) {
    m.unlock();
    return EBUSY;
  }

  readers++;
  m.unlock();
  return 0;
}

  
int dthread::mutex_rw::runlock() {
  m.lock();
  if (readers) {
    if (--readers == 0)
      c.signal();
    m.unlock();
    return 0;
  } else {
    m.unlock();
    return -1;
  }
}

int dthread::mutex_rw::wlock() {
  m.lock();
  while (readers || writers)
    c.wait(&m);
  writers++;
  m.unlock();
  return 0;
}

int dthread::mutex_rw::wlock_nb() {
  m.lock();

  if (readers || writers) {
    m.unlock();
    return EBUSY;
  }

  writers++;
  m.unlock();
  return 0;
}

int dthread::mutex_rw::wunlock() {
  m.lock();
  if (writers) {
    writers = 0;
    c.broadcast();
    m.unlock();
    return 0;
  } else {
    m.unlock();
    return -1;
  }
}
