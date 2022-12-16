// {HEADER}

#define _OBJECT_CC
#include "bom.h"

bom::object::object() {
  id = NULL;
}

bom::object::object(char *_id) {
  id = bom_os_strdup(_id);
}

bom::object::object(const bom::object &object) {
  id = bom_os_strdup(object.id);
}

bom::object::~object() {
  if (id)
    bom_os_free(id);
}

int bom::object::set_id(char *_id) {
  if (id)
    bom_os_free(id);
  id = _id ? bom_os_strdup(_id) : NULL;
  if (!id)
    return -1;
  return 0;
}

