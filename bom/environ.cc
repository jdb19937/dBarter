// {HEADER}

#define _ENVIRON_CC
#include "bom.h"

char *bom_db_directory = NULL;
void *(*bom_os_malloc)(size_t) = malloc;
void (*bom_os_free)(void *) = free;
void *(*bom_os_realloc)(void *, size_t) = realloc;
char *(*bom_os_strdup)(char *) = (char *(*)(char *))strdup;

void *operator new(size_t size) {
  void *addr = bom_os_malloc(size);
  //printf("bom: new %d (%d)\n", addr, size);
  return addr;
}

void operator delete(void *addr) {
  //printf("bom: delete %d\n", addr);
  bom_os_free(addr);
}

void bom_db_init(char *_db_root, int threaded) {
  bom_db_directory = bom_os_strdup(_db_root);
  db_init(threaded);
}

void bom_db_exit() {
  db_exit();
}

int bom_jump_set(char *which, void *func) {
  dmath_jump_set(which, func);

  if (!strcmp(which, "malloc")) {
    (void *&)bom_os_malloc = func;
    db_jump_set(func, DB_FUNC_MALLOC);
    return 0;
  }
  if (!strcmp(which, "free")) {
    (void *&)bom_os_free = func;
    db_jump_set(func, DB_FUNC_FREE);
    return 0;
  }
  if (!strcmp(which, "realloc")) {
    (void *&)bom_os_realloc = func;
    db_jump_set(func, DB_FUNC_REALLOC);
    return 0;
  }
  if (!strcmp(which, "strdup")) {
    (void *&)bom_os_strdup = func;
    return 0;
  }

  return -1;
}
