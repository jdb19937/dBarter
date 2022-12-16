// {HEADER}

#define _CONDITION_CC
#include "dthread.h"

dthread::condition::condition(...) {
  pthread_cond_init(&_c, NULL);
}
  
dthread::condition::~condition() {
  pthread_cond_destroy(&_c);
}

int dthread::condition::signal() {
  int result;
  result = pthread_cond_signal(&_c);
  return result;
}

int dthread::condition::broadcast() {
  int result;
  result = pthread_cond_broadcast(&_c);
  return result;
}

int dthread::condition::wait(dthread::mutex *m) {
  int result;
  result = pthread_cond_wait(&_c, &m->_m);
  return result;
}
