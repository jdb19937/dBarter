// {HEADER}

#ifndef _DX_H
#define _DX_H

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
}

#include <dthread.h>

extern int encode_command(int, char **, char **);
extern int decode_command(char *, int *, char ***);

struct dx {
  struct x;
  struct log;

  typedef u_int32_t x_id;
  
  typedef enum {
    x_null = 0,
    x_begun = 1,
    x_committed = 2,
    x_aborted = 3,
  } x_state;
  
  struct x {
    dx::log *log;
    x_id id;
    x_state state;

    x(dx::log *);
    ~x();
    
    int begin();
    int commit();
    int abort();
    
    int _write_begin();
    int _write_commit();
    int _write_abort();
  };

  struct log {
    char *path;
    int fdr, fdw;

    x_state *state;
    int state_length;
    x_id counter;

    log();
    ~log();
    
    int lock_r();
    int lock_r_nb();
    int unlock_r();
    int lock_rw();
    int lock_rw_nb();
    int unlock_rw();
    
    int open(char *, int _mode = 0600);
    int close();
    int is_open();
    int write(char *);
    int write(int, char **);
    int read(char **);
    int read(int *, char ***);
    int sync();
    
    int x_begin(x_id);
    int x_abort(x_id);
    int x_commit(x_id);
    x_id new_x_id();
  };
};

#endif
