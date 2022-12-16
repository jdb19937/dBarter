// {HEADER}

#define _ENVIRON_CC
#include "bif.h"

void _null_function() { }
void (*bif_shutdown)() = _null_function;

int bif_jump_set(char *which, void *func) {
  if (!strcmp(which, "shutdown")) {
    (void *&)bif_shutdown = func;
    return 0;
  }

  return -1;
}
