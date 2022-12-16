// {HEADER}

#include <errno.h>

#define _LOG_CC
#include "dx.h"

dx::log::log() {
  path = NULL;
  fdr = -1;
  fdw = -1;

  counter = 0;
  state = NULL;
  state_length = 0;
}

dx::log::~log() {
  close();

  if (state)
    free(state);
}

int dx::log::open(char *_path, int _mode) {
  if (path)
    close();
  fdr = ::open(_path, O_RDONLY | O_CREAT, _mode);
  if (fdr < 0)
    return fdr;
  fdw = ::open(_path, O_RDWR | O_CREAT | O_APPEND | O_SYNC, _mode);
  if (fdw < 0)
    return fdw;
  path = strdup(_path);
  
  sync();
  return 0;
}

int dx::log::close() {
  if (path) {
    free(path);
    path = NULL;
  }
  if (fdr >= 0)
    ::close(fdr);
  if (fdw >= 0)
    ::close(fdw);
  return 0;
}

int dx::log::lock_r() {
  return flock(fdr, LOCK_SH);
}
int dx::log::lock_r_nb() {
  return flock(fdr, LOCK_SH | LOCK_NB);
}
int dx::log::unlock_r() {
  return flock(fdr, LOCK_UN);
}

int dx::log::lock_rw() {
  return flock(fdw, LOCK_EX);
}
int dx::log::lock_rw_nb() {
  return flock(fdw, LOCK_EX | LOCK_NB);
}
int dx::log::unlock_rw() {
  return flock(fdw, LOCK_UN);
}

int dx::log::write(char *buffer) {
  int buffer_length = strlen(buffer);
  int result = ::write(fdw, buffer, buffer_length);
  if (result != buffer_length)
    return result;
  ::fsync(fdw);
  
  return 0;
}

int dx::log::write(int argc, char **argv) {
  char *buffer;
  int buffer_length;
  
  encode_command(argc, argv, &buffer);
  buffer_length = strlen(buffer) + 1;
  buffer[buffer_length - 1] = '\n';
  buffer[buffer_length] = '\0';
  
  return write(buffer);
}

int dx::log::read(char **bufferp) {
  int length, total = 0, n;

  *bufferp = (char *)malloc(length = 4096 + 8);
  
  while (n = ::read(fdr, *bufferp + total, 4096)) {
    *(*bufferp + total + n) = '\0';
    if (char *p = strchr(*bufferp + total, '\n')) {
      total += n;
      *p = '\0';
      lseek(fdr, ((int)p - (int)*bufferp) - total + 1, SEEK_CUR);
      break;
    } else if (total + n + 4096 > length) {
      *bufferp = (char *)realloc(*bufferp, length += 4096);
    }
    total += n;
  }
  
  if (!n) {
    lseek(fdr, -total, SEEK_CUR);
    free(*bufferp);
    return EAGAIN;
  }

  return 0;
}

int dx::log::read(int *argc, char ***argv) {
  char *buffer;

  if (int result = read(&buffer))
    return result;
  if (int result = decode_command(buffer, argc, argv)) {
    free(buffer);
    return result;
  }
  
  free(buffer);
  return 0;
}


int _state_grow(dx::log *log, dx::x_id id) {
  if (log->state_length == 0) {
    log->state = (dx::x_state *)malloc(sizeof(dx::x_state) * 64);
    log->state_length = 64;
    for (int i = log->state_length - 1; i >= 0; i--)
      log->state[i] = dx::x_null;
  } else if (log->state_length <= id) {
    int new_state_length = log->state_length * 2;
    if (new_state_length <= id)
      new_state_length = id + 1;
    log->state = (dx::x_state *)realloc(log->state,
     sizeof(dx::x_state) * new_state_length);
    for (int i = new_state_length - 1; i >= log->state_length; i--)
      log->state[i] = dx::x_null;
    log->state_length = new_state_length;
  } 
  
  return 0;
}

int dx::log::x_begin(dx::x_id id) {
  _state_grow(this, id);
  if (id >= counter)
    counter = id + 1;
  state[id] = dx::x_begun;
  return 0;
}

int dx::log::x_abort(dx::x_id id) {
  _state_grow(this, id);
  if (id >= counter)
    counter = id + 1;
  state[id] = dx::x_aborted;
  return 0;
}

int dx::log::x_commit(dx::x_id id) {
  _state_grow(this, id);
  if (id >= counter)
    counter = id + 1;
  state[id] = dx::x_committed;
  return 0;
}

int dx::log::sync() {
  dx::x_id x_first = 0;

  for (char *buffer = NULL; !read(&buffer); free(buffer)) {
    int argc;
    char **argv;
    
    if (int result = decode_command(buffer, &argc, &argv))
      return result;

    if (argc == 2 && !strcmp(argv[0], "begin")) {
      if (atoi(argv[1]) < x_first)
	x_first = atoi(argv[1]);
      x_begin(atoi(argv[1]));
    } else if (argc == 2 && !strcmp(argv[0], "abort")) {
      if (atoi(argv[1]) < x_first)
	x_first = atoi(argv[1]);
      x_abort(atoi(argv[1]));
    } else if (argc == 2 && !strcmp(argv[0], "commit")) {
      if (atoi(argv[1]) < x_first)
	x_first = atoi(argv[1]);
      x_commit(atoi(argv[1]));
    }

    for (int i = 0; i < argc; i++)
      free(argv[i]);
    if (argv)
      free(argv);
  }
  
  for (dx::x_id i = x_first; i < counter; i++) {
    if (state[i] == dx::x_begun) {
      dx::x x(this);
      x.state = dx::x_begun;
      x.id = i;
      x.abort();
    }
  }
}

dx::x_id dx::log::new_x_id() {
  return counter++;
}
