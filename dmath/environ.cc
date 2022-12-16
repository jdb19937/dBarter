// {HEADER}

#define _ENVIRON_CC
#include "dmath.h"

void *(*dmath_os_malloc)(size_t) = malloc;
void (*dmath_os_free)(void *) = free;
void *(*dmath_os_realloc)(void *, size_t) = realloc;
char *(*dmath_os_strdup)(char *) = (char *(*)(char *))strdup;

int dmath_jump_set(char *which, void *func) {
  if (!strcmp(which, "malloc")) {
    (void *&)dmath_os_malloc = func;
    return 0;
  }
  if (!strcmp(which, "free")) {
    (void *&)dmath_os_free = func;
    return 0;
  }
  if (!strcmp(which, "realloc")) {
    (void *&)dmath_os_realloc = func;
    return 0;
  }
  if (!strcmp(which, "strdup")) {
    (void *&)dmath_os_strdup = func;
    return 0;
  }

  return -1;
}
