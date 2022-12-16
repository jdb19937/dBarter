// {HEADER}

#define _COMMON_CC
#include "bif.h"

char *hash_extract(char *_hash, char *key, char **value) {
  char *hash;
  char *result = NULL;
  char *p, *q;
  
  hash = bom_os_strdup(_hash);
  p = hash;

  while (1) {
    if (!(q = strchr(p, ' '))) {
      result = NULL;
      goto done;
    }

    *q++ = '\0';
    
    if (strcmp(p, key)) {
      while (*q && *q != '\n')
	q++;
      if (*q)
	q++;
      p = q;
      continue;
    }
    
    p = q;
    while (*p && *p != '\n')
      p++;
    if (*p) {
      *p++ = '\0';
      result = _hash + ((int)p - (int)hash);
    } else
      result = _hash + ((int)p - (int)hash);

    *value = bom_os_strdup(q);
    goto done;
  }
  
done:
  bom_os_free(hash);
  return result;
}

int decode_oct(char **srcp, char *cp) {
  char *src = *srcp;

  if (src[0] < '0' || src[0] > '3')
    return -1;
  if (src[1] < '0' || src[1] > '7')
    return -1;
  if (src[2] < '0' || src[2] > '7')
    return -1;

  *cp = (src[2] - '0') + 010 * (src[1] - '0') + 0100 * (src[0] - '0');

  *srcp += 3;
  return 0;
}

int encode_hex(char c, char **srcp) {
  char *src = *srcp;
  char *table = "0123456789abcdef";
  
  *src++ = table[(c & 0xf0) >> 4];
  *src++ = table[c & 0x0f];
  
  *srcp = src;
  
  return 0;
}

int decode_hex(char **srcp, char *cp) {
  char *src = *srcp;

  *cp = 0;

  if (src[0] >= '0' && src[0] <= '9')
    *cp += (src[0] - '0') * 0x10;
  else if (src[0] >= 'a' && src[0] <= 'f')
    *cp += (src[0] + 10 - 'a') * 0x10;
  else if (src[0] >= 'A' && src[0] <= 'F')
    *cp += (src[0] + 10 - 'A') * 0x10;
  else
    return -1;

  if (src[1] >= '0' && src[1] <= '9')
    *cp += (src[1] - '0');
  else if (src[1] >= 'a' && src[1] <= 'f')
    *cp += (src[1] + 10 - 'a');
  else if (src[1] >= 'A' && src[1] <= 'F')
    *cp += (src[1] + 10 - 'A');
  else
    return -1;
  
  *srcp += 2;
  return 0;
}

int decode_character(char **srcp, char *cp, int *quoteq) {
  char c;
  
begin:
  c = **srcp;
  if (*quoteq) {
    if (!isprint(c))
      return -1;
  } else {
    if (c && !isprint(c))
      return -1;
    if (!c || isspace(c)) {
      *cp = '\0';
      while (isspace(**srcp))
	(*srcp)++;
      return 0;
    }
  }

  switch (c) {
  case ' ':
  case '\f':
  case '\n':
  case '\r':
  case '\t':
  case '\v':
    if (!*quoteq) {
      while (isspace(**srcp))
	(*srcp)++;
      *cp = '\0';
      return 0;
    }
    *cp = c;
    (*srcp)++;
    return 0;
  case '"':
    *quoteq = 1 - *quoteq;
    (*srcp)++;
    goto begin;
  case '\\':
    if (*quoteq) {
      (*srcp)++;
      switch (**srcp) {
      case '"':
        c = '"';
	break;
      case '\\':
	c = '\\';
	break;
      case 'f':
	c = '\f';
	break;
      case 'n':
	c = '\n';
	break;
      case 'r':
	c = '\r';
	break;
      case 't':
	c = '\t';
	break;
      case 'a':
	c = '\a';
	break;
      case 'b':
	c = '\b';
	break;
      case 'v':
	c = '\v';
	break;
      case '0':
      case '1':
      case '2':
      case '3':
	if (int result = decode_oct(srcp, cp))
	  return result;
	return 0;
      case 'x':
	(*srcp)++;
	if (int result = decode_hex(srcp, cp))
	  return result;
	return 0;
      default:
	return -1;
      }
    } else {
      *cp = c;
      (*srcp)++;
      return 0;
    }
  default:
    *cp = c;
    (*srcp)++;
    return 0;
  }
}

int decode_argument(char **srcp, char **argp) {
  char c;
  int quoteq = 0;
  int size0 = 256, i = 0, size;
  char *arg;

  while (isspace(**srcp))
    (*srcp)++;

  arg = *argp = new char[(size = size0) + 1];
  
  do {
    if (int result = decode_character(srcp, &c, &quoteq)) {
      delete *argp;
      return result;
    }
    
    if (i >= size)
      arg = *argp = (char *)realloc(arg, sizeof(char) * ((size <<= 1) + 1));

    arg[i++] = c;
  } while (c);
  
  while (isspace(**srcp))
    (*srcp)++;
  
  return 0;
}

int exists_argument(char **srcp) {
  while (isspace(**srcp))
    (*srcp)++;
  return **srcp ? 1 : 0;
}

int decode_string(char *src, int *argcp, char ***argvp) {
  struct argl_t {
    char *arg;
    argl_t *next;

    argl_t(char *_arg, argl_t **arglp) {
      arg = _arg;
      next = *arglp;
      *arglp = this;
    }

    ~argl_t() {
      if (next)
	delete next;
    }
  } *argl = NULL;

  *argcp = 0;
  
  
  while (isspace(*src))
    src++;
  while (*src) {
    char *arg = NULL;

    if (int result = decode_argument(&src, &arg)) {
      if (argl)
	delete argl;
      return result;
    }
    
    if (*argcp >= 256) {
      if (argl)
	delete argl;
      return -1;
    }

    new argl_t(arg, &argl);
    (*argcp)++;
  }
  
  *argvp = new char *[*argcp + 1];
  int argc = *argcp;
  
  (*argvp)[argc] = NULL;
  for (argl_t *argle = argl; argle; argle = argle->next) {
    argc--;
    (*argvp)[argc] = new char[strlen(argle->arg) + 1];
    strcpy((*argvp)[argc], argle->arg);
  }
  
  return 0;
}

int contains_metacharacters(char *src) {
  while (*src) {
    if (isspace(*src) || *src == '"' || !isprint(*src))
      return 1;
    src++;
  }
  return 0;
}

int encode_string(int argc, char **argv, char **srcp) {
  int length = 0;
  char *src;
  
  for (int i = 0; i < argc; i++)
    length += strlen(argv[i]) * 4 + 3;
  src = *srcp = new char[length + 1];
  
  for (int i = 0; i < argc; i++) {
    char *arg = argv[i];
    
    if (!*arg || contains_metacharacters(arg)) {
      src += sprintf(src, "\"");
      while (*arg) {
        switch (*arg) {
        case '\a':
          src += sprintf(src, "\\a");
          break;
        case '\n':
          src += sprintf(src, "\\n");
          break;
        case '\f':
          src += sprintf(src, "\\f");
          break;
        case '\v':
          src += sprintf(src, "\\v");
          break;
        case '\b':
          src += sprintf(src, "\\b");
          break;
        case '\t':
          src += sprintf(src, "\\t");
          break;
        case '\r':
          src += sprintf(src, "\\r");
          break;
        case '"':
          src += sprintf(src, "\\\"");
          break;
        case '\\':
          src += sprintf(src, "\\\\");
          break;
        default:
          if (isprint(*arg))
            *src++ = *arg;
          else {
	    src += sprintf(src, "\\x");
            encode_hex(*arg, &src);
	  }
          break;
        }
        arg++;
      }
      
      *src++ = '"';
    } else {
      src += sprintf(src, "%s", arg);
    }
     
    if (i < argc - 1)
      src += sprintf(src, " ");
  }
  
  *src = '\0';
  return 0;
}

