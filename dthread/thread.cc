// {HEADER}

#define _THREAD_CC
#include "dthread.h"

void thread_exit() {
  int retval;
  pthread_exit(&retval);
}

dthread::thread::thread(...) {

}

dthread::thread::thread(pthread_t __th) {
  _th = __th;
}

dthread::thread::~thread() {

}
  
int dthread::thread::spawn(thread_f f, void *a) {
  return pthread_create(&_th, NULL, f, a);
}
  
int dthread::thread::join(void *r) {
  return pthread_join(_th, &r);
}
  
int dthread::thread::detach() {
  return pthread_detach(_th);
}

int dthread::thread::cancel() {
  return pthread_cancel(_th);
}
  
dthread::thread thread_self() {
  dthread::thread t;
  t._th = pthread_self();
  return t;
}

int yield() {
  return sched_yield();
}
