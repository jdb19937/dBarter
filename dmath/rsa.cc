// {HEADER}
// really secure algorithm
// not for export from usa

#define _RSA_CC
#include "dmath.h"

int _primegen(dmath::bz* dest, unsigned long limbs) {
  do {
    dest->randomize(limbs);
  } while (!mpz_probab_prime_p(*dest, RSA_PRIMEGEN_CONFIDENCE_BITS >> 1));
  
  return 0;
}

int _relprimegen(dmath::bz* dest, const dmath::bz& src, unsigned long limbs) {
  dmath::bz n;

  do {
    dest->randomize(limbs);
    n = gcd(*dest, src);
  } while (n != 1);

  return 0;
}

int rsa_keygen(dmath::bz* kp, dmath::bz* ks, dmath::bz* km,
 unsigned long bits) {
  dmath::bz p, q, phin, n0;
  unsigned long limbs, slimbs;

  limbs = (bits >> 3) / sizeof(mp_limb_t);
  slimbs = limbs >> 1;
  if (slimbs <= 0)
    return -1;
  
  _primegen(&p, slimbs);
  _primegen(&q, slimbs);
  phin = (p - dmath::bz(1)) * (q - dmath::bz(1));

  do {
    _relprimegen(ks, phin, limbs);
  } while (!mpz_invert(n0, *ks, phin));

  *kp = n0 % phin;
  *km = p * q;
  
  return 0;
}

int rsa_challenge(dmath::bz* out, const dmath::bz& km) {
  int bits, limbs;
  dmath::bz x;
  
  bits = mpz_sizeinbase(km.z, 2);
  limbs = 1 + (bits >> 3) / sizeof(mp_limb_t);

  x.randomize(limbs + 1);
  *out = x % km;
  
  return 0;
}

int rsa_crypt(dmath::bz* out, const dmath::bz& _in,
 const dmath::bz& ke, const dmath::bz& km) {

  struct bz_list_t {
    dmath::bz n;
    bz_list_t *next;

    void *operator new(size_t size) {
      return dmath_os_malloc(sizeof(bz_list_t));
    }

    void operator delete(void *addr) {
      dmath_os_free(addr);
    }
  };

  dmath::bz in, in0, out0;
  bz_list_t *bz_list = NULL, *l, *next_l;

  for (in = _in; in > 0; in /= km) {
    in0 = in % km;
    mpz_powm(out0.z, in0.z, ke.z, km.z);
    bz_list_t *l = new bz_list_t;

    l->n = out0;
    l->next = bz_list;
    bz_list = l;
  }
  
  *out = 0;

  for (bz_list_t *l = bz_list; l; l = next_l) {
    next_l = l->next;
    *out *= km;
    *out += l->n;
    if (l)
      delete l;
  }
  
  return 0;
}

