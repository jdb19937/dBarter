// {HEADER}

#define _MESSAGE_CC
#include "bom.h"
#include "bif.h"

bif::message::message() {
  code = 0;
  desc = NULL;
  data = NULL;
  next = NULL;
}

bif::message::~message() {
  if (desc)
    bom_os_free(desc);
  if (data)
    bom_os_free(data);
  if (next)
    delete next;
}

int bif::message::set_desc(char *_desc) {
  if (desc)
    bom_os_free(desc);
  desc = _desc ? bom_os_strdup(_desc) : NULL;
  if (!desc)
    return -1;
  return 0;
}

int bif::message::set_data(char *_data) {
  if (data)
    bom_os_free(data);
  data = _data ? bom_os_strdup(_data) : NULL;
  if (!data)
    return -1;
  return 0;
}
