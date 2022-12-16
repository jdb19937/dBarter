// {HEADER}

#define _S_SOCKET_CC
#include "bom.h"
#include "bif.h"

bif::socket::socket() {
  s = -1;
};

bif::socket::~socket() {
  if (s >= 0)
    close();
}
  
int bif::socket::read(void *buf, size_t count) {
  return ::read(s, buf, count);
}
  
int bif::socket::write(void *buf, size_t count) {
  return ::write(s, buf, count);
}
  
int bif::socket::close() {
  return ::close(s);
}

void bif::socket::writef(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vdprintf(s, fmt, ap);
  va_end(ap);
}

int find_host(char *host, int port, struct sockaddr_in *sin) {
  in_addr addr;
  struct hostent *h;

  memset(sin, 0, sizeof(struct sockaddr_in));
  sin->sin_family = AF_INET;
  sin->sin_port = htons(port);
  
  if (*host) {
    if (!inet_aton(host, &addr)) {
      if (!(h = gethostbyname(host)))
        return -1;
      memcpy(&addr, h->h_addr, sizeof(in_addr));
    }
  } else {
    unsigned int any = htonl(INADDR_ANY);
    memcpy(&addr, &any, sizeof(in_addr));
  }

  memcpy(&sin->sin_addr, &addr, sizeof(in_addr));

  return 0;
}

int find_host(int port, struct sockaddr_in *sin) {
  return find_host("", port, sin);
}

void block_socket(int s, char on) {
  fcntl(s, F_SETFL, on ? 0 : O_NDELAY);
}
