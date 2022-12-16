#ifdef __cplusplus
extern "C" {
#endif
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#ifdef __cplusplus
}
#endif

int __eh_pc;
int __throw;
int terminate__Fv;

void *_gmp_malloc(size_t size) {
  return safemalloc(size);
}

void *_gmp_realloc(void *ptr, size_t old_size, size_t new_size) {
  return saferealloc(ptr, new_size);
}

void _gmp_free(void *ptr, size_t size) {
  free(ptr);
}

#include <dmath.h>

MODULE = dmath		PACKAGE = dmath		


int
_dmath_init()
	CODE:
	{
          extern void *savepv; /* fuzorx */
	  mp_set_memory_functions(_gmp_malloc, _gmp_realloc, _gmp_free);
	  dmath_jump_set("malloc", (void *)safemalloc);
	  dmath_jump_set("realloc", (void *)saferealloc);
	  dmath_jump_set("free", (void *)safefree);
	  dmath_jump_set("strdup", (void *)savepv);
	  RETVAL = 0;
	}
	OUTPUT:
	RETVAL

int
_rsa_keygen(s_kp, s_ks, s_km, bits)
	char *s_kp;
	char *s_ks;
	char *s_km;
	int bits;
	CODE:
	{
	  dmath::bz kp, ks, km;
	  RETVAL = (int)rsa_keygen(&kp, &ks, &km, bits);
	  bz_to_string(kp, &s_kp);
	  bz_to_string(ks, &s_ks);
	  bz_to_string(km, &s_km);
	}
	OUTPUT:
	RETVAL
	s_kp
	s_km
	s_ks


int
_rsa_crypt(s_c, s_p, s_ke, s_km)
	char *s_p;
	char *s_ke;
	char *s_km;
	char *s_c;
	CODE:
	{
	  dmath::bz p, ke, km, c;
	  string_to_bz(s_p, &p);
	  string_to_bz(s_ke, &ke);
	  string_to_bz(s_km, &km);
	  string_to_bz(s_c, &c);
	  RETVAL = (int)rsa_crypt(&c, p, ke, km);
	  bz_to_string(c, &s_c);
	}
	OUTPUT:
	RETVAL
	s_c

int
_rsa_challenge(s_c, s_km)
	char *s_c;
	char *s_km;
	CODE:
	{
	  dmath::bz km, c;
	  string_to_bz(s_km, &km);
	  string_to_bz(s_c, &c);
	  RETVAL = (int)rsa_challenge(&c, km);
	  bz_to_string(c, &s_c);
	}
	OUTPUT:
	RETVAL
	s_c
	
int
_bz_add(c, a, b)
	char *c;
	char *a;
	char *b;
	CODE:
	{
	  dmath::bz bc, ba, bb;
	  RETVAL = 0;
	  RETVAL |= string_to_bz(a, &ba);
	  RETVAL |= string_to_bz(b, &bb);
	  bc = ba + bb;
	  bz_to_string(bc, &c);
	}
	OUTPUT:
	RETVAL
	c

int
_bz_mul(c, a, b)
	char *c;
	char *a;
	char *b;
	CODE:
	{
	  dmath::bz bc, ba, bb;
	  RETVAL = 0;
	  RETVAL |= string_to_bz(a, &ba);
	  RETVAL |= string_to_bz(b, &bb);
	  bc = ba * bb;
	  bz_to_string(bc, &c);
	}
	OUTPUT:
	RETVAL
	c

int
_bz_sub(c, a, b)
	char *c;
	char *a;
	char *b;
	CODE:
	{
	  dmath::bz bc, ba, bb;
	  RETVAL = 0;
	  RETVAL |= string_to_bz(a, &ba);
	  RETVAL |= string_to_bz(b, &bb);
	  bc = ba - bb;
	  bz_to_string(bc, &c);
	}
	OUTPUT:
	RETVAL
	c

int
_bz_div(c, a, b)
	char *c;
	char *a;
	char *b;
	CODE:
	{
	  dmath::bz bc, ba, bb;
	  RETVAL = 0;
	  RETVAL |= string_to_bz(a, &ba);
	  RETVAL |= string_to_bz(b, &bb);
	  bc = ba / bb;
	  bz_to_string(bc, &c);
	}
	OUTPUT:
	RETVAL
	c

int
_bz_mod(c, a, b)
	char *c;
	char *a;
	char *b;
	CODE:
	{
	  dmath::bz bc, ba, bb;
	  RETVAL = 0;
	  RETVAL |= string_to_bz(a, &ba);
	  RETVAL |= string_to_bz(b, &bb);
	  bc = ba % bb;
	  bz_to_string(bc, &c);
	}
	OUTPUT:
	RETVAL
	c

int
_bz_pow(c, a, b)
	char *c;
	char *a;
	char *b;
	CODE:
	{
	  dmath::bz bc, ba;
	  RETVAL = 0;
	  RETVAL |= string_to_bz(a, &ba);
	  mpz_pow_ui(bc.z, ba.z, atoi(b));
	  bz_to_string(bc, &c);
	}
	OUTPUT:
	RETVAL
	c

int
_bq_add(c, a, b)
	char *c;
	char *a;
	char *b;
	CODE:
	{
	  dmath::bq bc, ba, bb;
	  RETVAL = 0;
	  RETVAL |= string_to_bq(a, &ba);
	  RETVAL |= string_to_bq(b, &bb);
	  bc = ba + bb;
	  bq_to_string(bc, &c);
	}
	OUTPUT:
	RETVAL
	c

int
_bq_sub(c, a, b)
	char *c;
	char *a;
	char *b;
	CODE:
	{
	  dmath::bq bc, ba, bb;
	  RETVAL = 0;
	  RETVAL |= string_to_bq(a, &ba);
	  RETVAL |= string_to_bq(b, &bb);
	  bc = ba - bb;
	  bq_to_string(bc, &c);
	}
	OUTPUT:
	RETVAL
	c

int
_bq_mul(c, a, b)
	char *c;
	char *a;
	char *b;
	CODE:
	{
	  dmath::bq bc, ba, bb;
	  RETVAL = 0;
	  RETVAL |= string_to_bq(a, &ba);
	  RETVAL |= string_to_bq(b, &bb);
	  bc = ba * bb;
	  bq_to_string(bc, &c);
	}
	OUTPUT:
	RETVAL
	c

int
_bq_div(c, a, b)
	char *c;
	char *a;
	char *b;
	CODE:
	{
	  dmath::bq bc, ba, bb;
	  RETVAL = 0;
	  RETVAL |= string_to_bq(a, &ba);
	  RETVAL |= string_to_bq(b, &bb);
	  bc = ba / bb;
	  bq_to_string(bc, &c);
	}
	OUTPUT:
	RETVAL
	c
