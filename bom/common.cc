// {HEADER}

#define _COMMON_CC
#include "bom.h"

int debugf(char *fmt, ...) {
#ifdef B_DEBUG
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
#endif
  
  return 0;
}

int warningf(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  
  return 0;
}

int errorf(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  
  return 0;
}

static char _sprintf_buffer[1 << 18];

int sprintf_alloc(char **s, char *fmt, ...) { 
  int length;

  va_list ap;
  va_start(ap, fmt);
  length = vsnprintf(_sprintf_buffer, 1 << 20 - 1, fmt, ap);
  va_end(ap);
  
  *s = (char *)bom_os_malloc(length + 1);
  strcpy(*s, _sprintf_buffer);
  
  return 0;
}
