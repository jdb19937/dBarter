// {HEADER}

#define _BQ_CC
#include "dmath.h"
  
dmath::bq::bq() {
  mpq_init(q);
  mpq_set_si(q, 0, 1);
}

dmath::bq::bq(signed long sin, signed long sid) {
  mpq_init(q);
  mpq_set_si(q, sin, sid);
  mpq_canonicalize(q);
}

dmath::bq::bq(mpq_t _q) {
  mpq_init(q);
  mpq_set(q, _q);
}

dmath::bq::bq(const dmath::bz &x) {
  mpq_init(q);
  mpq_set_z(q, x.z);
}

dmath::bq::bq(const dmath::bq &x) {
  mpq_init(q);
  mpq_set(q, x.q);
}

dmath::bq::~bq() {
  mpq_clear(q);
}

void dmath::bq::canonicalize() {
  mpq_canonicalize(q);
}

bool is_integer(const dmath::bq &q) {
  bool result;

  mpz_t one;
  mpz_init(one);
  mpz_set_ui(one, 1);
  result = mpz_cmp(mpq_denref(q.q), one);
  mpz_clear(one);

  return (result == 0);
}

dmath::bq& dmath::bq::operator=(const dmath::bz &x) {
  mpq_set_z(q, x.z);
  return *this;
}

dmath::bq& dmath::bq::operator=(const dmath::bq &x) {
  mpq_set(q, x.q);
  return *this;
}

dmath::bq& dmath::bq::operator=(signed long sin) {
  mpq_set_si(q, sin, 1);
  return *this;
}
  
dmath::bq& dmath::bq::operator+=(const dmath::bq &x) {
  mpq_add(q, q, x.q);
  mpq_canonicalize(q);
  return *this;
}

dmath::bq& dmath::bq::operator-=(const dmath::bq &x) {
  mpq_sub(q, q, x.q);
  mpq_canonicalize(q);
  return *this;
}

dmath::bq& dmath::bq::operator*=(const dmath::bq &x) {
  mpq_mul(q, q, x.q);
  mpq_canonicalize(q);
  return *this;
}

dmath::bq& dmath::bq::operator/=(const dmath::bq &x) {
  mpq_div(q, q, x.q);
  mpq_canonicalize(q);
  return *this;
}

dmath::bq operator+(const dmath::bq& x, const dmath::bq& y) {
  dmath::bq w;
  mpq_add(w.q, x.q, y.q);
  mpq_canonicalize(w.q);
  return w;
}

dmath::bq operator-(const dmath::bq& x, const dmath::bq& y) {
  dmath::bq w;
  mpq_sub(w.q, x.q, y.q);
  mpq_canonicalize(w.q);
  return w;
}

dmath::bq operator*(const dmath::bq& x, const dmath::bq& y) {
  dmath::bq w;
  mpq_mul(w.q, x.q, y.q);
  mpq_canonicalize(w.q);
  return w;
}

dmath::bq operator/(const dmath::bq& x, const dmath::bq& y) {
  dmath::bq w;
  mpq_div(w.q, x.q, y.q);
  mpq_canonicalize(w.q);
  return w;
}

dmath::bq operator-(const dmath::bq& x) {
  dmath::bq w;
  mpq_neg(w.q, x.q);
  return w;
}

bool operator==(const dmath::bq& x, const dmath::bq& y) {
  return mpq_equal(x.q, y.q);
}

bool operator==(const dmath::bq& x, unsigned long ui) {
  return mpq_cmp_ui(x.q, ui, 1) == 0;
}

bool operator==(unsigned long ui, const dmath::bq& x) {
  return mpq_cmp_ui(x.q, ui, 1) == 0;
}

bool operator!=(const dmath::bq& x, const dmath::bq& y) {
  return !mpq_equal(x.q, y.q);
}

bool operator!=(const dmath::bq& x, unsigned long ui) {
  return mpq_cmp_ui(x.q, ui, 1) != 0;
}

bool operator!=(unsigned long ui, const dmath::bq& x) {
  return mpq_cmp_ui(x.q, ui, 1) != 0;
}

bool operator<(const dmath::bq& x, const dmath::bq& y) {
  return mpq_cmp(x.q, y.q) < 0;
}

bool operator<(const dmath::bq& x, unsigned long ui) {
  return mpq_cmp_ui(x.q, ui, 1) < 0;
}

bool operator<(unsigned long ui, const dmath::bq& x) {
  return mpq_cmp_ui(x.q, ui, 1) > 0;
}

bool operator<=(const dmath::bq& x, const dmath::bq& y) {
  return mpq_cmp(x.q, y.q) <= 0;
}

bool operator<=(const dmath::bq& x, unsigned long ui) {
  return mpq_cmp_ui(x.q, ui, 1) <= 0;
}

bool operator<=(unsigned long ui, const dmath::bq& x) {
  return mpq_cmp_ui(x.q, ui, 1) >= 0;
}

bool operator>(const dmath::bq& x, const dmath::bq& y) {
  return mpq_cmp(x.q, y.q) > 0;
}

bool operator>(const dmath::bq& x, unsigned long ui) {
  return mpq_cmp_ui(x.q, ui, 1) > 0;
}

bool operator>(unsigned long ui, const dmath::bq& x) {
  return mpq_cmp_ui(x.q, ui, 1) < 0;
}

bool operator>=(const dmath::bq& x, const dmath::bq& y) {
  return mpq_cmp(x.q, y.q) >= 0;
}

bool operator>=(const dmath::bq& x, unsigned long ui) {
  return mpq_cmp_ui(x.q, ui, 1) >= 0;
}

bool operator>=(unsigned long ui, const dmath::bq& x) {
  return mpq_cmp_ui(x.q, ui, 1) <= 0;
}

int _to_decimal(const dmath::bz& _num, const dmath::bz &_den, dmath::bz *num, int *exp) {
  dmath::bz den(_den);
  *num = _num;

  while (den % 5 == 0) {
    (*exp)++;
    den /= 5;
    *num *= 2;
  }
  while (den % 2 == 0) {
    (*exp)++;
    den /= 2;
    *num *= 5;
  }
  
  return (den == 1) ? 0 : 1;
}

int bq_to_string(const dmath::bq &_q, char **sp) {
  char *s, *snum = 0, *sden = 0, *sz = 0, *sk = 0;
  bool neg = 0;
  int ns = 0, ds = 0, zs = 0, ks = 0;
  int exp = 0;
  dmath::bq q, q0;
  dmath::bz num, den;
  dmath::bz z, k, g;
  
  q = _q;
  if (q < 0) {
    dmath::bq neg1(-1, 1);
    q *= neg1;
    neg = 1;
  }

  z = q;

  if (z > 0) {
    bz_to_string(z, &sz);
    zs = strlen(sz) + 1;
  }

  q0 = z;
  q -= q0;

  mpz_set(num.z, mpq_numref(q.q));
  mpz_set(den.z, mpq_denref(q.q));
  
  if (!_to_decimal(num, den, &k, &exp)) {
    bz_to_string(k, &sk);
    ks = exp;
  } else if (num > 0) {
    bz_to_string(num, &snum);
    bz_to_string(den, &sden);
    ns = strlen(snum) + 1;
    ds = strlen(sden) + 1;
  }

  if (zs + ns + ds)
    s = (char *)dmath_os_malloc(neg + zs + ns + ds);
  else if (ks)
    s = (char *)dmath_os_malloc(neg + zs + 2 + ks + 1);
  else
    s = (char *)dmath_os_malloc(neg + 2);

  if (!s) {
    if (sk)
      free(sk);
    if (sz)
      free(sz);
    if (snum)
      free(snum);
    if (sden)
      free(sden);
    return -1;
  }

  if (zs && ns) 
    sprintf(s, "%s%s+%s/%s", neg ? "-" : "", sz, snum, sden);
  else if (ns)
    sprintf(s, "%s%s/%s", neg ? "-" : "", snum, sden);
  else if (zs && ks) {
    char buf[256];
    sprintf(buf, "%%s%%s.%%0%ds", ks);
    sprintf(s, buf, neg ? "-" : "", sz, sk);
  } else if (ks) {
    char buf[256];
    sprintf(buf, "%%s0.%%0%ds", ks);
    sprintf(s, buf, neg ? "-" : "", sk);
  } else if (zs)
    sprintf(s, "%s%s", neg ? "-" : "", sz);
  else
    sprintf(s, "0");

  if (sk)
    free(sk);
  if (sz)
    free(sz);
  if (snum)
    free(snum);
  if (sden)
    free(sden);

  *sp = s;
  return 0;
}

int string_to_bq(char *_sp, dmath::bq *q) {
  char *sp;
  dmath::bz z, w, x;
  dmath::bq r;
  bool neg = 0;

  if (*_sp == '-') {
    neg = 1;
    _sp++;
  }

  sp = strdup(_sp);

  if (char *p = strchr(sp, '.')) {
    *p++ = '\0';
    mpz_set_str(z, sp, 10);
    mpq_set_z(*q, z);
    mpz_set_str(w, p, 10);
    mpq_set_z(r, w);
    for (int i = strlen(p); i > 0; i--)
      r /= 10;
    *q += r;
  } else if (char *p = strchr(sp, '+')) {
    char *r;
    dmath::bq q0;

    if (!(r = strchr(p, '/'))) {
      free(sp);
      return -1;
    }

    *p++ = '\0';
    *r++ = '\0';
    
    if (string_to_bz(r, &x)) {
      free(sp);
      return -1;
    }
    if (x == 0) {
      free(sp);
      return -1;
    }
    if (string_to_bz(sp, &z)) {
      free(sp);
      return -1;
    }
    if (string_to_bz(p, &w)) {
      free(sp);
      return -1;
    }
    
    mpq_set_z(*q, z);
    mpq_set_num(q0, w);
    mpq_set_den(q0, x);
    mpq_canonicalize(q0);
    *q += q0;
  } else if (char *p = strchr(sp, '/')) {
    *p++ = '\0';

    if (string_to_bz(sp, &z)) {
      free(sp);
      return -1;
    }
    if (string_to_bz(p, &w)) {
      free(sp);
      return -1;
    }
    mpq_set_num(*q, z);
    mpq_set_den(*q, w);
    mpq_canonicalize(*q);
  } else {
    if (string_to_bz(sp, &z)) {
      free(sp);
      return -1;
    }
      
    mpq_set_z(*q, z);
  }
  
  free(sp);

  if (neg) {
    dmath::bq neg1(-1, 1);
    *q *= neg1;
  }

  return 0;
}

int dbt_to_bq(const DBT& d, dmath::bq *qp) {
  return string_to_bq((char *)d.data, qp);
}

int bq_to_dbt(const dmath::bq& q, DBT *dp) {
  int result;
  memset(dp, 0, sizeof(DBT));
  result = bq_to_string(q, (char **)&dp->data);
  if (result)
    return result;
  dp->size = 1 + strlen((char *)dp->data);
  dp->flags = DB_DBT_MALLOC;
  return 0;
}
