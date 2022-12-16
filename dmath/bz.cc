// {HEADER}

#define _BZ_CC
#include "dmath.h"

dmath::bz::bz() {
  mpz_init_set_ui(z, 0);
}

dmath::bz::bz(mpz_t _z) {
  memcpy(&z, &_z, sizeof(mpz_t));
}

dmath::bz::bz(signed long si) {
  mpz_init_set_si(z, si);
}

dmath::bz::bz(const dmath::bz &x) {
  mpz_init_set(z, x.z);
}

dmath::bz::bz(const dmath::bq &x) {
  mpz_init(z);
  mpz_set_q(z, x.q);
}

dmath::bz::~bz() {
  mpz_clear(z);
}

dmath::bz& dmath::bz::operator=(const dmath::bz &x) {
  mpz_set(z, x.z);
  return *this;
}

dmath::bz& dmath::bz::operator=(const dmath::bq &x) {
  mpz_set_q(z, x.q);
  return *this;
}

dmath::bz& dmath::bz::operator=(signed long si) {
  mpz_set_si(z, si);
  return *this;
}

dmath::bz &dmath::bz::operator+=(const dmath::bz &x) {
  mpz_add(z, z, x.z);
  return *this;
}

dmath::bz &dmath::bz::operator+=(unsigned long ui) {
  mpz_add_ui(z, z, ui);
  return *this;
}

dmath::bz &dmath::bz::operator-=(const dmath::bz &x) {
  mpz_sub(z, z, x.z);
  return *this;
}

dmath::bz &dmath::bz::operator-=(unsigned long ui) {
  mpz_sub_ui(z, z, ui);
  return *this;
}

dmath::bz &dmath::bz::operator*=(const dmath::bz &x) {
  mpz_mul(z, z, x.z);
  return *this;
}

dmath::bz &dmath::bz::operator*=(unsigned long ui) {
  mpz_mul_ui(z, z, ui);
  return *this;
}

dmath::bz &dmath::bz::operator/=(const dmath::bz &x) {
  mpz_fdiv_q(z, z, x.z);
  return *this;
}

dmath::bz &dmath::bz::operator/=(unsigned long ui) {
  mpz_fdiv_q_ui(z, z, ui);
  return *this;
}

dmath::bz &dmath::bz::operator%=(const dmath::bz &x) {
  mpz_mod(z, z, x.z);
  return *this;
}

dmath::bz &dmath::bz::operator%=(unsigned long ui) {
  mpz_mod_ui(z, z, ui);
  return *this;
}

static bool _tested_dev_random = 0;
static FILE *_dev_random = NULL;

dmath::bz &dmath::bz::randomize(unsigned long limbs) {
  if (!_tested_dev_random) {
    _tested_dev_random = 1;
    if (!(_dev_random = fopen("/dev/random", "r")))
      goto randomize1;
    if (fcntl(fileno(_dev_random), F_SETFL, O_NONBLOCK) < 0) {
      fclose(_dev_random);
      _dev_random = NULL;
      goto randomize1;
    }
  }
randomize1:
  
  mp_limb_t *buffer = (mp_limb_t *)dmath_os_malloc(limbs * sizeof(mp_limb_t));
  int length = 0;
 
  if (_tested_dev_random && _dev_random)
    length = fread(buffer, sizeof(mp_limb_t), limbs, _dev_random);
  for (int i = length; i < limbs; i++)
    buffer[i] = random();
  
  z->_mp_d = buffer;
  z->_mp_alloc = limbs;
  z->_mp_size = limbs;

  return *this;
}

dmath::bz operator+(const dmath::bz& x, const dmath::bz& y) {
  dmath::bz w;
  mpz_add(w.z, x.z, y.z);
  return w;
}

dmath::bz operator+(const dmath::bz& x, unsigned long ui) {
  dmath::bz w;
  mpz_add_ui(w.z, x.z, ui);
  return w;
}

dmath::bz operator+(unsigned long ui, const dmath::bz& x) {
  dmath::bz w;
  mpz_add_ui(w.z, x.z, ui);
  return w;
}

dmath::bz operator-(const dmath::bz& x, const dmath::bz& y) {
  dmath::bz w;
  mpz_sub(w.z, x.z, y.z);
  return w;
}

dmath::bz operator-(const dmath::bz& x, unsigned long ui) {
  dmath::bz w;
  mpz_sub_ui(w.z, x.z, ui);
  return w;
}

dmath::bz operator-(unsigned long ui, const dmath::bz& x) {
  dmath::bz w(ui);
  mpz_sub(w.z, w.z, x.z);
  return w;
}

dmath::bz operator*(const dmath::bz& x, const dmath::bz& y) {
  dmath::bz w;
  mpz_mul(w.z, x.z, y.z);
  return w;
}

dmath::bz operator*(const dmath::bz& x, unsigned long ui) {
  dmath::bz w;
  mpz_mul_ui(w.z, x.z, ui);
  return w;
}

dmath::bz operator*(unsigned long ui, const dmath::bz& x) {
  dmath::bz w;
  mpz_mul_ui(w.z, x.z, ui);
  return w;
}

dmath::bz operator/(const dmath::bz& x, const dmath::bz& y) {
  dmath::bz w;
  mpz_fdiv_q(w.z, x.z, y.z);
  return w;
}

dmath::bz operator/(const dmath::bz& x, unsigned long ui) {
  dmath::bz w;
  mpz_fdiv_q_ui(w.z, x.z, ui);
  return w;
}

dmath::bz operator/(unsigned long ui, const dmath::bz& x) {
  dmath::bz w(ui);
  mpz_fdiv_q(w.z, w.z, x.z);
  return w;
}

dmath::bz operator%(const dmath::bz& x, const dmath::bz& y) {
  dmath::bz w;
  mpz_mod(w.z, x.z, y.z);
  return w;
}

dmath::bz operator%(const dmath::bz& x, unsigned long ui) {
  dmath::bz w;
  mpz_mod_ui(w.z, x.z, ui);
  return w;
}

dmath::bz operator%(unsigned long ui, const dmath::bz& x) {
  dmath::bz w(ui);
  mpz_mod(w.z, w.z, x.z);
  return w;
}

dmath::bz operator-(const dmath::bz& x) {
  dmath::bz w;
  mpz_neg(w.z, x.z);
  return w;
}

bool operator==(const dmath::bz &x, const dmath::bz &y) {
  return mpz_cmp(x.z, y.z) == 0;
}

bool operator==(const dmath::bz &x, signed long si) {
  return mpz_cmp_si(x.z, si) == 0;
}
  
bool operator==(signed long si, const dmath::bz& x) {
  return mpz_cmp_si(x.z, si) == 0;
}
  
bool operator!=(const dmath::bz &x, const dmath::bz &y) {
  return mpz_cmp(x.z, y.z) != 0;
}

bool operator!=(const dmath::bz &x, signed long si) {
  return mpz_cmp_si(x.z, si) != 0;
}
  
bool operator!=(signed long si, const dmath::bz& x) {
  return mpz_cmp_si(x.z, si) != 0;
}
  
bool operator<(const dmath::bz &x, const dmath::bz &y) {
  return mpz_cmp(x.z, y.z) < 0;
}

bool operator<(const dmath::bz &x, signed long si) {
  return mpz_cmp_si(x.z, si) < 0;
}
  
bool operator<(signed long si, const dmath::bz& x) {
  return mpz_cmp_si(x.z, si) > 0;
}
  
bool operator>(const dmath::bz &x, const dmath::bz &y) {
  return mpz_cmp(x.z, y.z) > 0;
}

bool operator>(const dmath::bz &x, signed long si) {
  return mpz_cmp_si(x.z, si) > 0;
}
  
bool operator>(signed long si, const dmath::bz& x) {
  return mpz_cmp_si(x.z, si) < 0;
}
  
bool operator<=(const dmath::bz &x, const dmath::bz &y) {
  return mpz_cmp(x.z, y.z) <= 0;
}

bool operator<=(const dmath::bz &x, signed long si) {
  return mpz_cmp_si(x.z, si) <= 0;
}
  
bool operator<=(signed long si, const dmath::bz& x) {
  return mpz_cmp_si(x.z, si) >= 0;
}
  
bool operator>=(const dmath::bz &x, const dmath::bz &y) {
  return mpz_cmp(x.z, y.z) >= 0;
}

bool operator>=(const dmath::bz &x, signed long si) {
  return mpz_cmp_si(x.z, si) >= 0;
}

dmath::bz gcd(const dmath::bz &a, const dmath::bz &b) {
  dmath::bz c;
  mpz_gcd(c.z, a.z, b.z);
  return c;
}

dmath::bz lcm(const dmath::bz &a, const dmath::bz &b) {
  dmath::bz c, d;
  mpz_gcd(c.z, a.z, b.z);
  mpz_set(d.z, a.z);
  mpz_mul(d.z, d.z, b.z);
  mpz_fdiv_q(d.z, d.z, c.z);
  return d;
}
  
bool operator>=(signed long si, const dmath::bz& x) {
  return mpz_cmp_si(x.z, si) <= 0;
}

int string_to_bz(char *s, dmath::bz *zp) {
  return mpz_set_str(zp->z, s, 10);
}

int bz_to_string(const dmath::bz &z, char **sp) {
  if (!(*sp = mpz_get_str(NULL, 10, z.z)))
    return -1;
  return 0;
}

int dbt_to_bz(const DBT& d, dmath::bz* zp) {
  return string_to_bz((char *)d.data, zp);
}

int bz_to_dbt(const dmath::bz& z, DBT *dp) {
  int result;
  memset(dp, 0, sizeof(DBT));
  result = bz_to_string(z, (char **)&dp->data);
  if (result)
    return result;
  dp->size = 1 + strlen((char *)dp->data);
  dp->flags = DB_DBT_MALLOC;
  return 0;
}
