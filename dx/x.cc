// {HEADER}

#define _X_CC
#include "dx.h"

dx::x::x(dx::log *_log) {
  id = (x_id)-1;
  log = _log;
  state = dx::x_null;
}

dx::x::~x() {

}

int dx::x::_write_begin() {
  char buffer[256];
  sprintf(buffer, "begin %d\n", id);
  return log->write(buffer);
}

int dx::x::_write_abort() {
  char buffer[256];
  sprintf(buffer, "abort %d\n", id);
  return log->write(buffer);
}

int dx::x::_write_commit() {
  char buffer[256];
  sprintf(buffer, "commit %d\n", id);
  return log->write(buffer);
}

int dx::x::begin() {
  if (state == dx::x_begun)
    abort();
  state = dx::x_begun;
  id = log->new_x_id();
  if (int result = _write_begin())
    return result;
  return 0;
}

int dx::x::commit() {
  if (state != dx::x_begun)
    return -1;
  state = dx::x_committed;
  if (int result = _write_commit())
    return result;
  return 0;
}

int dx::x::abort() {
  if (state != dx::x_begun)
    return -1;
  state = dx::x_aborted;
  if (int result = _write_abort())
    return result;
  return 0;
}
