// {HEADER}

#ifndef _DMATH_H
#define _DMATH_H

extern "C" {
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <gmp.h>
#include <db.h>
}

struct dmath {
  struct bz;
  struct bq;

  struct bz {
    mpz_t z;
    
    bz();
    bz(mpz_t _z);
    bz(signed long si);
    bz(const bz &x);
    bz(const bq &x);
  
    ~bz();
    
    bz &randomize(unsigned long limbs);
  
    bz& operator=(const bz &x);
    bz& operator=(const bq &x);
    bz& operator=(signed long si);
    
    bz& operator+=(const bz &x);
    bz& operator+=(unsigned long ui);
    bz& operator-=(const bz &x);
    bz& operator-=(unsigned long ui);
    bz& operator*=(const bz &x);
    bz& operator*=(unsigned long ui);
    bz& operator/=(const bz &x);
    bz& operator/=(unsigned long ui);
    bz& operator%=(const bz &x);
    bz& operator%=(unsigned long ui);
    
    inline operator mpz_t&() { return z; } 
    
    friend bz operator+(const bz& x, const bz& y);
    friend bz operator+(const bz& x, unsigned long ui);
    friend bz operator+(unsigned long ui, const bz& x);
    friend bz operator-(const bz& x, const bz& y);
    friend bz operator-(const bz& x, unsigned long ui);
    friend bz operator-(unsigned long ui, const bz& x);
    friend bz operator*(const bz& x, const bz& y);
    friend bz operator*(const bz& x, unsigned long ui);
    friend bz operator*(unsigned long ui, const bz& x);
    friend bz operator/(const bz& x, const bz& y);
    friend bz operator/(const bz& x, unsigned long ui);
    friend bz operator/(unsigned long ui, const bz& x);
    friend bz operator%(const bz& x, const bz& y);
    friend bz operator%(const bz& x, unsigned long ui);
    friend bz operator%(unsigned long ui, const bz& x);
    friend bz operator-(const bz& x);
    friend bool operator==(const bz &x, const bz &y);
    friend bool operator==(const bz &x, signed long si);
    friend bool operator==(signed long si, const bz &x);
    friend bool operator!=(const bz &x, const bz &y);
    friend bool operator!=(const bz &x, signed long si);
    friend bool operator!=(signed long si, const bz &x);
    friend bool operator<(const bz &x, const bz &y);
    friend bool operator<(const bz &x, signed long si);
    friend bool operator<(signed long si, const bz &x);
    friend bool operator>(const bz &x, const bz &y);
    friend bool operator>(const bz &x, signed long si);
    friend bool operator>(signed long si, const bz &x);
    friend bool operator<=(const bz &x, const bz &y);
    friend bool operator<=(const bz &x, signed long si);
    friend bool operator<=(signed long si, const bz &x);
    friend bool operator>=(const bz &x, const bz &y);
    friend bool operator>=(const bz &x, signed long si);
    friend bool operator>=(signed long si, const bz &x);
    
    friend bz gcd(const bz &, const bz &);
    friend bz lcm(const bz &, const bz &);
  
    friend int string_to_bz(char *, bz *);
    friend int bz_to_string(const bz &, char **);
    friend int dbt_to_bz(const DBT& d, bz *);
    friend int bz_to_dbt(const bz &, DBT *);
  };
  
  struct bq {
    mpq_t q;
    
    bq();
    bq(mpq_t _q);
    bq(signed long sin, signed long sid = 1);
    bq(const bz &x);
    bq(const bq &q);
  
    ~bq();
  
    bq& operator=(const bz &x);
    bq& operator=(const bq &x);
    bq& operator=(signed long sin);
    
    bq& operator+=(const bq &x);
    bq& operator-=(const bq &x);
    bq& operator*=(const bq &x);
    bq& operator/=(const bq &x);
    
    void canonicalize();
    friend bool is_integer(const bq &);
  
    inline operator mpq_t&() { return q; } 
    
    friend bq operator+(const bq& x, const bq& y);
    friend bq operator-(const bq& x, const bq& y);
    friend bq operator*(const bq& x, const bq& y);
    friend bq operator/(const bq& x, const bq& y);
    friend bq operator-(const bq& x);
    friend bool operator==(const bq &x, const bq &y);
    friend bool operator==(const bq &x, unsigned long ui);
    friend bool operator==(unsigned long ui, const bq &x);
    friend bool operator!=(const bq &x, const bq &y);
    friend bool operator!=(const bq &x, unsigned long ui);
    friend bool operator!=(unsigned long ui, const bq &x);
    friend bool operator<(const bq &x, const bq &y);
    friend bool operator<(const bq &x, unsigned long ui);
    friend bool operator<(unsigned long ui, const bq &x);
    friend bool operator>(const bq &x, const bq &y);
    friend bool operator>(const bq &x, unsigned long ui);
    friend bool operator>(unsigned long ui, const bq &x);
    friend bool operator<=(const bq &x, const bq &y);
    friend bool operator<=(const bq &x, unsigned long ui);
    friend bool operator<=(unsigned long ui, const bq &x);
    friend bool operator>=(const bq &x, const bq &y);
    friend bool operator>=(const bq &x, unsigned long ui);
    friend bool operator>=(unsigned long ui, const bq &x);
  
    friend int string_to_bq(char *, bq *);
    friend int bq_to_string(const bq &, char **);
    friend int dbt_to_bq(const DBT &, bq *);
    friend int bq_to_dbt(const bq &, DBT *);
  };
};
  
#define RSA_PRIMEGEN_CONFIDENCE_BITS	32
  
int rsa_keygen(dmath::bz *, dmath::bz *, dmath::bz *, unsigned long);
int rsa_challenge(dmath::bz *, const dmath::bz &);
int rsa_crypt(dmath::bz *, const dmath::bz &,
 const dmath::bz &, const dmath::bz &);

int dmath_jump_set(char *, void *);

#ifndef _ENVIRON_CC
extern void *(*dmath_os_malloc)(size_t);
extern void (*dmath_os_free)(void *);
extern void *(*dmath_os_realloc)(void *, size_t);
extern char *(*dmath_os_strdup)(char *);
#endif

#endif
