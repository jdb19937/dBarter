// {HEADER}

#define _VALUE_CC
#include "bom.h"

bom::value::value() {
  quant = 0;
  next = NULL;
}

bom::value::value(const bom::denom &_denom, const dmath::bq& _quant) {
  denom.set_id(_denom.id);
  quant = _quant;
  next = NULL;
}

bom::value::value(const bom::value& v) {
  denom.set_id(v.denom.id);
  quant = v.quant;
  next = NULL;
}

bom::value::~value() {
  if (next)
    delete next;
}

int bom::value::set_denom(const bom::denom &_denom) {
  return denom.set_id(_denom.id);
}

int bom::value::set_quant(const dmath::bq &_quant) {
  quant = _quant;
}

bom::value &bom::value::operator=(const bom::value& v) {
  denom.set_id(v.denom.id);
  quant = v.quant;
  next = NULL;
  return *this;
}

int string_to_value(char *_s, bom::value *vp) {
  char *p0, *p1, *s;
  int result;
  
  if (!(s = bom_os_strdup(_s)))
    return -1;

  if (!(p0 = strchr(s, '{'))) {
    bom_os_free(s);
    return -1;
  }
  *p0++ = '\0';

  if (!(p1 = strchr(p0, '}'))) {
    bom_os_free(s);
    return -1;
  }
  *p1++ = '\0';

  if (*p1) {
    bom_os_free(s);
    return -1;
  }

  result = string_to_bq(p0, &vp->quant);
  if (result) {
    bom_os_free(s);
    return result;
  }

  result = vp->denom.set_id(s);
  if (result) {
    bom_os_free(s);
    return result;
  }
  
  bom_os_free(s);
  return 0;
}

int value_to_string(const bom::value &v, char **sp) {
  int result, has_space;
  char *squant;
 
  result = bq_to_string(v.quant, &squant);
  if (result)
    return result;

  *sp = (char *)bom_os_malloc(strlen(v.denom.id) + strlen(squant) + 5);
  has_space = strchr(squant, ' ') ? 1 : 0;
  
  sprintf(*sp, "%s{%s%s%s}",
   v.denom.id, has_space ? "\"" : "", squant, has_space ? "\"" : "");
  bom_os_free(squant);
  
  return 0;
}

int portfolio_to_string(bom::value *vl, char **sp) {
  int length = 0, count = 0;
  
  struct value_string_t {
    char *svalue;
    value_string_t *next;
  };

  if (vl) {
    value_string_t *vsl = NULL;

    for (bom::value *v = vl; v; v = v->next) {
      value_string_t *vs = new value_string_t;
      value_to_string(*v, &vs->svalue);
      length += strlen(vs->svalue) + 1;
      vs->next = vsl;
      vsl = vs;
    }

    *sp = (char *)bom_os_malloc(length);

    value_string_t *next_vs, *vs;

    vs = vsl;
    next_vs = vs->next;
    count += sprintf(*sp + count, "%s", vs->svalue);
    bom_os_free(vs->svalue);
    delete vs;

    for (vs = next_vs; vs; vs = next_vs) {
      next_vs = vs->next;
      count += sprintf(*sp + count, " %s", vs->svalue);
      bom_os_free(vs->svalue);
      delete vs;
    }
  } else {
    *sp = (char *)bom_os_malloc(1);
    **sp = '\0';
  }
  
  return 0;
}

int string_to_portfolio(char *_s, bom::value **vlp) {
  char *s, *p, *q;
  int result;
  
  q = s = bom_os_strdup(_s);
  *vlp = NULL;

  while (p = strchr(q, ' ')) {
    *p++ = '\0';

    bom::value *vp = new bom::value;
    if (result = string_to_value(q, vp)) {
      if (*vlp)
        delete *vlp;
      delete vp;
      bom_os_free(s);
      return result;
    }
    vp->next = *vlp;
    *vlp = vp;
    
    while (isspace(*p))
      p++;
    q = p;
  }

  bom::value *vp = new bom::value;
  if (result = string_to_value(q, vp)) {
    if (*vlp)
      delete *vlp;
    delete vp;
    bom_os_free(s);
    return result;
  }
  vp->next = *vlp;
  *vlp = vp;
  
  bom_os_free(s);
  return 0;
}
