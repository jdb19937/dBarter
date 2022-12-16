// {HEADER}

#define _MUTEX_CC
#include "dthread.h"

dthread::mutex::mutex(...) {
  pthread_mutex_init(&_m, NULL);
}
  
dthread::mutex::~mutex() {
  pthread_mutex_destroy(&_m);
}

int dthread::mutex::lock() {
  return pthread_mutex_lock(&_m);
}

int dthread::mutex::lock_nb() {
  return pthread_mutex_trylock(&_m);
}
  
int dthread::mutex::unlock() {
  return pthread_mutex_unlock(&_m);
}
