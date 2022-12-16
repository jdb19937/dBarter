// {HEADER}

#define _SITE_CC
#include "bif.h"

bif::site::site() {
  host = NULL;
  port = DEFAULT_PORT;
  next = NULL;
}

bif::site::site(char *_host) {
  host = _host ? bom_os_strdup(_host) : NULL;
  port = DEFAULT_PORT;
  next = NULL;
}

bif::site::site(char *_host, unsigned short _port) {
  host = _host ? bom_os_strdup(_host) : NULL;
  port = _port;
  next = NULL;
}

bif::site::site(const bif::site& site) {
  host = site.host ? bom_os_strdup(site.host) : NULL;
  port = site.port;
  next = NULL;
}

bif::site::~site() {
  if (host)
    bom_os_free(host);
  if (next)
    delete next;
}

int bif::site::set_host(char *_host) {
  if (host)
    bom_os_free(host);
  host = _host ? bom_os_strdup(_host) : NULL;
  if (!host)
    return -1;
  return 0;
}

int bif::site::set_port(unsigned short _port) {
  port = _port;
  return 0;
}

bif::site &bif::site::operator=(const bif::site &site) {
  set_host(site.host);
  set_port(site.port);
  return *this;
}

char operator==(const bif::site &a, const bif::site &b) {
  if (!a.host || !b.host)
    return (a.host == b.host && a.port == b.port);
  else
    return (!strcmp(a.host, b.host) && a.port == b.port);
}

char operator!=(const bif::site &a, const bif::site &b) {
  if (!a.host || !b.host)
    return (a.host != b.host || a.port != b.port);
  else
    return (strcmp(a.host, b.host) || a.port != b.port);
}

int site_to_string(const bif::site &site, char **sp) {
  if (!site.host)
    return -1;
  if (site.port != DEFAULT_PORT) {
    if (!(*sp = (char *)bom_os_malloc(strlen(site.host) + 7)))
      return -1;
    sprintf(*sp, "%s:%hd", site.host, site.port);
  } else {
    if (!(*sp = bom_os_strdup(site.host)))
      return -1;
  }

  return 0;
}

int string_to_site(char *s, bif::site *sp) {
  if (char *p = strchr(s, ':')) {
    int length = (int)p - (int)s;
    if (sp->host)
      bom_os_free(sp->host);
    if (!(sp->host = (char *)bom_os_malloc(length + 1)))
      return -1;
    memcpy(sp->host, s, length);
    *(sp->host + length) = '\0';
    sp->set_port(atoi(p + 1));
  } else {
    sp->set_host(s);
    sp->set_port(DEFAULT_PORT);
  }
  
  return 0;
}

int string_to_remote(char *s, char **sp, bif::site *sitep) {
  if (char *p = strchr(s, '@')) {
    int length = (int)p - (int)s;
    if (!(*sp = (char *)bom_os_malloc(length + 1)))
      return -1;
    memcpy(*sp, s, length);
    *(*sp + length) = '\0';
    if (int result = string_to_site(p + 1, sitep))
      return result;
  } else {
    if (!(*sp = bom_os_strdup(s)))
      return -1;
    if (sitep->host)
      bom_os_free(sitep->host);
    sitep->host = NULL;
    sitep->port = 0;
  }

  return 0;
}

int remote_to_string(char *s, const bif::site &site, char **sp) {
  if (site.host) {
    char *ssite;
    int length;

    if (int result = site_to_string(site, &ssite))
      return result;
    length = strlen(s) + 1 + strlen(ssite);
    if (!(*sp = (char *)bom_os_malloc(length + 1))) {
      bom_os_free(ssite);
      return -1;
    }
    sprintf(*sp, "%s@%s", s, ssite);
  } else {
    if (!(*sp = bom_os_strdup(s)))
      return -1;
  }

  return 0;
}

