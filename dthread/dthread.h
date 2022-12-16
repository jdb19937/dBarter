// {HEADER}

#ifndef _DTHREAD_H
#define _DTHREAD_H

extern "C" {
#include <pthread.h>
}

typedef void *(*thread_f)(void *);

extern void thread_exit();

struct dthread {
  struct thread {
    pthread_t _th;
    
    thread(...);
    thread(pthread_t);
    ~thread();
    
    int spawn(thread_f f, void *a);
    int join(void *r);
    int detach();
    int cancel();
  
    friend thread thread_self();
    friend int yield();
  };
  
  struct mutex {
    pthread_mutex_t _m;
    
    mutex(...);
    ~mutex();
    int lock();
    int lock_nb();
    int unlock();
  };
  
  struct condition {
    pthread_cond_t _c;
    
    condition(...);
    ~condition();
  
    int signal();
    int broadcast();
    int wait(mutex *);
  };
  
  struct mutex_rw {
    mutex m;
    condition c;
    int readers, writers;
    
    mutex_rw(...);
    ~mutex_rw();
    int rlock();
    int rlock_nb();
    int runlock();
    int wlock();
    int wlock_nb();
    int wunlock();
  };
};
  
#endif
