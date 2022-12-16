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

#include <bom.h>

void *_gmp_malloc(size_t size) {
  return safemalloc(size);
}

void *_gmp_realloc(void *ptr, size_t old_size, size_t new_size) {
  return saferealloc(ptr, new_size);
}

void _gmp_free(void *ptr, size_t size) {
  free(ptr);
}

MODULE = bom		PACKAGE = bom		

int
_bom_init()
	CODE:
	{
          extern void *savepv; //fuzorx
	  mp_set_memory_functions(_gmp_malloc, _gmp_realloc, _gmp_free);
	  bom_jump_set("malloc", (void *)safemalloc);
	  bom_jump_set("realloc", (void *)saferealloc);
	  bom_jump_set("free", (void *)safefree);
	  bom_jump_set("strdup", (void *)savepv);
	  RETVAL = 0;
	}
	OUTPUT:
	RETVAL

int
_bom_db_init(db_dir)
	char *db_dir;
	CODE:
	{
	  bom_db_init(db_dir, 0);
	  RETVAL = 0;
	}
	OUTPUT:
	RETVAL

int
_bom_db_exit()
	CODE:
	{
	  bom_db_exit();
	  RETVAL = 0;
	}
	OUTPUT:
	RETVAL
	
int
_bom_db_get(file, _txn, key, val)
	char *file;
	char *_txn;
	char *key;
	char *val;
	CODE:
	{
	  txn_t *txn = (txn_t *)atoi(_txn);
	  RETVAL = bom_db_get(file, txn, key, &val);
	}
	OUTPUT:
	RETVAL
	val

int
_bom_db_del(file, _txn, key)
	char *file;
	char *_txn;
	char *key;
	CODE:
	{
	  txn_t *txn = (txn_t *)atoi(_txn);
	  RETVAL = bom_db_del(file, txn, key);
	}
	OUTPUT:
	RETVAL

int
_bom_db_put(file, _txn, key, val)
	char *file;
	char *_txn;
	char *key;
	char *val;
	CODE:
	{
	  txn_t *txn = (txn_t *)atoi(_txn);
	  RETVAL = bom_db_put(file, txn, key, val);
	}
	OUTPUT:
	RETVAL


int
_txn_new(_txn)
	char *_txn;
	CODE:
	{
	  txn_t *txn = NULL;
	  RETVAL = txn_new(&txn);
          _txn = (char *)bom_os_malloc(32);
	  sprintf(_txn, "%ld", (unsigned long)txn);
	}
	OUTPUT:
	RETVAL
	_txn

int
_txn_commit(_txn)
	char *_txn;
	CODE:
	{
	  txn_t *txn = (txn_t *)atoi(_txn);
	  RETVAL = txn_commit(txn);
	}
	OUTPUT:
	RETVAL

int
_txn_abort(_txn)
	char *_txn;
	CODE:
	{
	  txn_t *txn = (txn_t *)atoi(_txn);
	  RETVAL = txn_abort(txn);
	}
	OUTPUT:
	RETVAL


int
_acct_exists(_acct, _txn, exists)
	char *_acct;
	unsigned int _txn;
	char exists;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = acct.exists(txn, &exists);
	}
	OUTPUT:
	RETVAL
	exists


int
_acct_get_promise(_acct, _txn, promise)
	char *_acct;
	unsigned int _txn;
	char *promise;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = acct.get_promise(txn, &promise);
	}
	OUTPUT:
	RETVAL
	promise

int
_acct_set_promise(_acct, _txn, promise)
	char *_acct;
	unsigned int _txn;
	char *promise;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = acct.set_promise(txn, promise);
	}
	OUTPUT:
	RETVAL

int
_acct_del_promise(_acct, _txn)
	char *_acct;
	unsigned int _txn;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = acct.del_promise(txn);
	}
	OUTPUT:
	RETVAL

int
_acct_get_public(_acct, _txn, _kpublic)
	char *_acct;
	unsigned int _txn;
	char *_kpublic;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  dmath::bz kpublic;
	  RETVAL = acct.get_public(txn, &kpublic);
	  bz_to_string(kpublic, &_kpublic);
	}
	OUTPUT:
	RETVAL
	_kpublic

int
_acct_set_public(_acct, _txn, _kpublic)
	char *_acct;
	unsigned int _txn;
	char *_kpublic;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  dmath::bz kpublic;
	  string_to_bz(_kpublic, &kpublic);
	  RETVAL = acct.set_public(txn, kpublic);
	}
	OUTPUT:
	RETVAL

int
_acct_del_public(_acct, _txn)
	char *_acct;
	unsigned int _txn;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = acct.del_public(txn);
	}
	OUTPUT:
	RETVAL

int
_acct_get_secret(_acct, _txn, _ksecret)
	char *_acct;
	unsigned int _txn;
	char *_ksecret;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  dmath::bz ksecret;
	  RETVAL = acct.get_secret(txn, &ksecret);
	  bz_to_string(ksecret, &_ksecret);
	}
	OUTPUT:
	RETVAL
	_ksecret

int
_acct_set_secret(_acct, _txn, _ksecret)
	char *_acct;
	unsigned int _txn;
	char *_ksecret;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  dmath::bz ksecret;
	  string_to_bz(_ksecret, &ksecret);
	  RETVAL = acct.set_secret(txn, ksecret);
	}
	OUTPUT:
	RETVAL

int
_acct_del_secret(_acct, _txn)
	char *_acct;
	unsigned int _txn;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = acct.del_secret(txn);
	}
	OUTPUT:
	RETVAL

int
_acct_get_modulus(_acct, _txn, _kmodulus)
	char *_acct;
	unsigned int _txn;
	char *_kmodulus;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  dmath::bz kmodulus;
	  RETVAL = acct.get_modulus(txn, &kmodulus);
	  bz_to_string(kmodulus, &_kmodulus);
	}
	OUTPUT:
	RETVAL
	_kmodulus

int
_acct_set_modulus(_acct, _txn, _kmodulus)
	char *_acct;
	unsigned int _txn;
	char *_kmodulus;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  dmath::bz kmodulus;
	  string_to_bz(_kmodulus, &kmodulus);
	  RETVAL = acct.set_modulus(txn, kmodulus);
	}
	OUTPUT:
	RETVAL

int
_acct_del_modulus(_acct, _txn)
	char *_acct;
	unsigned int _txn;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = acct.del_modulus(txn);
	}
	OUTPUT:
	RETVAL

int
_acct_get_flags(_acct, _txn, flags)
	char *_acct;
	unsigned int _txn;
	unsigned int flags;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = acct.get_flags(txn, &flags);
	}
	OUTPUT:
	RETVAL
	flags

int
_acct_set_flags(_acct, _txn, flags)
	char *_acct;
	unsigned int _txn;
	unsigned int flags;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = acct.set_flags(txn, flags);
	}
	OUTPUT:
	RETVAL

int
_acct_del_flags(_acct, _txn)
	char *_acct;
	unsigned int _txn;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = acct.del_flags(txn);
	}
	OUTPUT:
	RETVAL


int
_acct_get_portfolio(_acct, _txn, _vl)
	char *_acct;
	unsigned int _txn;
	char *_vl;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  bom::value *vl = NULL;
	  RETVAL = acct.get_portfolio(txn, &vl);
	  if (!RETVAL)
	    portfolio_to_string(vl, &_vl);
	  if (vl)
	    delete vl;
	}
	OUTPUT:
	RETVAL
	_vl

int
_acct_set_portfolio(_acct, _txn, _vl)
	char *_acct;
	unsigned int _txn;
	char *_vl;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  bom::value *vl = NULL;
	  string_to_portfolio(_vl, &vl);
	  RETVAL = acct.set_portfolio(txn, vl);
	  if (vl)
	    delete vl;
	}
	OUTPUT:
	RETVAL

int
_acct_del_portfolio(_acct, _txn)
	char *_acct;
	unsigned int _txn;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = acct.del_portfolio(txn);
	}
	OUTPUT:
	RETVAL

int
_acct_get_note_list(_acct, _txn, _nl)
	char *_acct;
	unsigned int _txn;
	char *_nl;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  bom::note *nl = NULL;
	  RETVAL = acct.get_note_list(txn, &nl);
	  if (!RETVAL)
	    note_list_to_string(nl, &_nl);
	  if (nl)
	    delete nl;
	}
	OUTPUT:
	RETVAL
	_nl

int
_acct_del_note_list(_acct, _txn)
	char *_acct;
	unsigned int _txn;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = acct.del_note_list(txn);
	}
	OUTPUT:
	RETVAL

int
_acct_get_denom_list(_acct, _txn, _dl)
	char *_acct;
	unsigned int _txn;
	char *_dl;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  bom::denom *dl = NULL;
	  RETVAL = acct.get_denom_list(txn, &dl);
	  if (!RETVAL)
	    denom_list_to_string(dl, &_dl);
	  if (dl)
	    delete dl;
	}
	OUTPUT:
	RETVAL
	_dl

int
_acct_get_event_list(_acct, _txn, _el)
	char *_acct;
	unsigned int _txn;
	char *_el;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  bom::event *el = NULL;
	  RETVAL = acct.get_event_list(txn, &el);
	  if (!RETVAL)
	    event_list_to_string(el, &_el);
	  if (el)
	    delete el;
	}
	OUTPUT:
	RETVAL
	_el

int
_acct_get_offer_list(_acct, _txn, _ol)
	char *_acct;
	unsigned int _txn;
	char *_ol;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  bom::offer *ol = NULL;
	  RETVAL = acct.get_offer_list(txn, &ol);
	  if (!RETVAL)
	    offer_list_to_string(ol, &_ol);
	  if (ol)
	    delete ol;
	}
	OUTPUT:
	RETVAL
	_ol

int
_acct_destroy(_acct, _txn)
	char *_acct;
	unsigned int _txn;
	CODE:
	{
	  bom::acct acct(_acct);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = bom_db_del("acct_passwd.db", txn, acct.id);
          if (!RETVAL)
	    RETVAL = acct.destroy(txn);
	}
	OUTPUT:
	RETVAL

int
_get_acct_list(_txn, _ul)
	unsigned int _txn;
	char *_ul;
	CODE:
	{
	  txn_t *txn = (txn_t *)_txn;
	  bom::acct *ul = NULL;
	  RETVAL = get_acct_list(txn, &ul);
	  if (!RETVAL)
	    acct_list_to_string(ul, &_ul);
	  if (ul)
	    delete ul;
	}
	OUTPUT:
	RETVAL
	_ul

int
_note_exists(_note, _txn, exists)
	char *_note;
	unsigned int _txn;
	char exists;
	CODE:
	{
	  bom::note note(_note);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = note.exists(txn, &exists);
	}
	OUTPUT:
	RETVAL
	exists


int
_note_get_challenge(_note, _txn, _challenge)
	char *_note;
	unsigned int _txn;
	char *_challenge;
	CODE:
	{
	  bom::note note(_note);
	  txn_t *txn = (txn_t *)_txn;
	  dmath::bz challenge;
	  RETVAL = note.get_challenge(txn, &challenge);
	  bz_to_string(challenge, &_challenge);
	}
	OUTPUT:
	RETVAL
	_challenge

int
_note_set_challenge(_note, _txn, _challenge)
	char *_note;
	unsigned int _txn;
	char *_challenge;
	CODE:
	{
	  bom::note note(_note);
	  txn_t *txn = (txn_t *)_txn;
	  dmath::bz challenge;
	  string_to_bz(_challenge, &challenge);
	  RETVAL = note.set_challenge(txn, challenge);
	}
	OUTPUT:
	RETVAL

int
_note_del_challenge(_note, _txn)
	char *_note;
	unsigned int _txn;
	CODE:
	{
	  bom::note note(_note);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = note.del_challenge(txn);
	}
	OUTPUT:
	RETVAL

int
_note_get_issuer(_note, _txn, _issuer)
	char *_note;
	unsigned int _txn;
	char *_issuer;
	CODE:
	{
	  bom::note note(_note);
	  txn_t *txn = (txn_t *)_txn;
	  bom::acct issuer(_issuer);
	  RETVAL = note.get_issuer(txn, &issuer);
	  _issuer = strdup(issuer.id);
	}
	OUTPUT:
	RETVAL
	_issuer

int
_note_set_issuer(_note, _txn, _issuer)
	char *_note;
	unsigned int _txn;
	char *_issuer;
	CODE:
	{
	  bom::note note(_note);
	  txn_t *txn = (txn_t *)_txn;
	  bom::acct issuer(_issuer);
	  RETVAL = note.set_issuer(txn, issuer);
	}
	OUTPUT:
	RETVAL

int
_note_del_issuer(_note, _txn)
	char *_note;
	unsigned int _txn;
	CODE:
	{
	  bom::note note(_note);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = note.del_issuer(txn);
	}
	OUTPUT:
	RETVAL
	
	
int
_note_get_portfolio(_note, _txn, _vl)
	char *_note;
	unsigned int _txn;
	char *_vl;
	CODE:
	{
	  bom::note note(_note);
	  txn_t *txn = (txn_t *)_txn;
	  bom::value *vl;
	  RETVAL = note.get_portfolio(txn, &vl);
	  portfolio_to_string(vl, &_vl);
	  delete vl;
	}
	OUTPUT:
	RETVAL
	_vl

int
_note_set_portfolio(_note, _txn, _vl)
	char *_note;
	unsigned int _txn;
	char *_vl;
	CODE:
	{
	  bom::note note(_note);
	  txn_t *txn = (txn_t *)_txn;
	  bom::value *vl;
	  string_to_portfolio(_vl, &vl);
	  RETVAL = note.set_portfolio(txn, vl);
	  delete vl;
	}
	OUTPUT:
	RETVAL

int
_note_del_portfolio(_note, _txn)
	char *_note;
	unsigned int _txn;
	CODE:
	{
	  bom::note note(_note);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = note.del_portfolio(txn);
	}
	OUTPUT:
	RETVAL

int
_note_destroy(_note, _txn)
	char *_note;
	unsigned int _txn;
	CODE:
	{
	  bom::note note(_note);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = note.destroy(txn);
	}
	OUTPUT:
	RETVAL

	
int
_denom_exists(_denom, _txn, exists)
	char *_denom;
	unsigned int _txn;
	char exists;
	CODE:
	{
	  bom::denom denom(_denom);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = denom.exists(txn, &exists);
	}
	OUTPUT:
	RETVAL
	exists


int
_denom_get_issuer(_denom, _txn, _issuer)
	char *_denom;
	unsigned int _txn;
	char *_issuer;
	CODE:
	{
	  bom::denom denom(_denom);
	  txn_t *txn = (txn_t *)_txn;
	  bom::acct issuer(_issuer);
	  RETVAL = denom.get_issuer(txn, &issuer);
	  _issuer = strdup(issuer.id);
	}
	OUTPUT:
	RETVAL
	_issuer

int
_denom_set_issuer(_denom, _txn, _issuer)
	char *_denom;
	unsigned int _txn;
	char *_issuer;
	CODE:
	{
	  bom::denom denom(_denom);
	  txn_t *txn = (txn_t *)_txn;
	  bom::acct issuer(_issuer);
	  RETVAL = denom.set_issuer(txn, issuer);
	}
	OUTPUT:
	RETVAL

int
_denom_del_issuer(_denom, _txn)
	char *_denom;
	unsigned int _txn;
	CODE:
	{
	  bom::denom denom(_denom);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = denom.del_issuer(txn);
	}
	OUTPUT:
	RETVAL

int
_denom_get_promise(_denom, _txn, promise)
	char *_denom;
	unsigned int _txn;
	char *promise;
	CODE:
	{
	  bom::denom denom(_denom);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = denom.get_promise(txn, &promise);
	}
	OUTPUT:
	RETVAL
	promise

int
_denom_set_promise(_denom, _txn, promise)
	char *_denom;
	unsigned int _txn;
	char *promise;
	CODE:
	{
	  bom::denom denom(_denom);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = denom.set_promise(txn, promise);
	}
	OUTPUT:
	RETVAL

int
_denom_del_promise(_denom, _txn)
	char *_denom;
	unsigned int _txn;
	CODE:
	{
	  bom::denom denom(_denom);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = denom.del_promise(txn);
	}
	OUTPUT:
	RETVAL

int
_denom_destroy(_denom, _txn)
	char *_denom;
	unsigned int _txn;
	CODE:
	{
	  bom::denom denom(_denom);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = denom.destroy(txn);
	}
	OUTPUT:
	RETVAL

int
_event_exists(_event, _txn, exists)
	unsigned int _event;
	unsigned int _txn;
	char exists;
	CODE:
	{
	  bom::event event(_event);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = event.exists(txn, &exists);
	}
	OUTPUT:
	RETVAL
	exists


int
_event_get_type(_event, _txn, _type)
	unsigned int _event;
	unsigned int _txn;
	char *_type;
	CODE:
	{
	  bom::event event(_event);
	  txn_t *txn = (txn_t *)_txn;
	  bom::event_type type;
	  RETVAL = event.get_type(txn, &type);
	  if (!RETVAL)
	    _type = strdup(event_name[type]);
	}
	OUTPUT:
	RETVAL
	_type

int
_event_set_type(_event, _txn, _type)
	unsigned int _event;
	unsigned int _txn;
	char *_type;
	CODE:
	{
	  bom::event event(_event);
	  bom::event_type type;
	  txn_t *txn = (txn_t *)_txn;

	  int i;
	  for (i = 0; *event_name[i]; i++)
	    if (!strcmp(event_name[i], _type)) {
	      type = i;
	      break;
	    }
	  if (*event_name[type])
	    RETVAL = event.set_type(txn, type);
	  else
	    RETVAL = -1;
	}
	OUTPUT:
	RETVAL

int
_event_del_type(_event, _txn)
	unsigned int _event;
	unsigned int _txn;
	CODE:
	{
	  bom::event event(_event);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = event.del_type(txn);
	}
	OUTPUT:
	RETVAL

int
_event_get_acct_list(_event, _txn, _ul)
	unsigned int _event;
	unsigned int _txn;
	char *_ul;
	CODE:
	{
	  bom::event event(_event);
	  txn_t *txn = (txn_t *)_txn;
	  bom::acct *ul = NULL;
	  RETVAL = event.get_acct_list(txn, &ul);
	  if (!RETVAL) {
	    acct_list_to_string(ul, &_ul);
	    if (ul)
	      delete ul;
	  }
	}
	OUTPUT:
	RETVAL
	_ul

int
_event_set_acct_list(_event, _txn, _ul)
	unsigned int _event;
	unsigned int _txn;
	char *_ul;
	CODE:
	{
	  bom::event event(_event);
	  txn_t *txn = (txn_t *)_txn;
	  bom::acct *ul = NULL;

	  string_to_acct_list(_ul, &ul);
	  RETVAL = event.set_acct_list(txn, ul);
	}
	OUTPUT:
	RETVAL

int
_event_del_acct_list(_event, _txn)
	unsigned int _event;
	unsigned int _txn;
	CODE:
	{
	  bom::event event(_event);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = event.del_acct_list(txn);
	}
	OUTPUT:
	RETVAL

int
_event_get_data(_event, _txn, data)
	unsigned int _event;
	unsigned int _txn;
	char *data;
	CODE:
	{
	  bom::event event(_event);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = event.get_data(txn, &data);
	}
	OUTPUT:
	RETVAL
	data

int
_event_set_data(_event, _txn, data)
	unsigned int _event;
	unsigned int _txn;
	char *data;
	CODE:
	{
	  bom::event event(_event);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = event.set_data(txn, data);
	}
	OUTPUT:
	RETVAL

int
_event_del_data(_event, _txn)
	unsigned int _event;
	unsigned int _txn;
	CODE:
	{
	  bom::event event(_event);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = event.del_data(txn);
	}
	OUTPUT:
	RETVAL

int
_event_destroy(_event, _txn)
	unsigned int _event;
	unsigned int _txn;
	CODE:
	{
	  bom::event event(_event);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = event.destroy(txn);
	}
	OUTPUT:
	RETVAL

int
_get_last_event(_txn, _event)
	unsigned int _txn;
	unsigned int _event;
	CODE:
	{
	  txn_t *txn = (txn_t *)_txn;
	  bom::event event;
	  RETVAL = get_last_event(txn, &event);
	  _event = event.id;
	}
	OUTPUT:
	RETVAL
	_event

int
_offer_exists(_offer, _txn, exists)
	char *_offer;
	unsigned int _txn;
	char exists;
	CODE:
	{
	  bom::offer offer(_offer);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = offer.exists(txn, &exists);
	}
	OUTPUT:
	RETVAL
	exists

int
_offer_destroy(_offer, _txn)
	char *_offer;
	unsigned int _txn;
	CODE:
	{
	  bom::offer offer(_offer);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = offer.destroy(txn);
	}
	OUTPUT:
	RETVAL


int
_offer_get_issuer(_offer, _txn, _issuer)
	char *_offer;
	unsigned int _txn;
	char *_issuer;
	CODE:
	{
	  bom::offer offer(_offer);
	  txn_t *txn = (txn_t *)_txn;
	  bom::acct issuer(_issuer);
	  RETVAL = offer.get_issuer(txn, &issuer);
	  _issuer = strdup(issuer.id);
	}
	OUTPUT:
	RETVAL
	_issuer

int
_offer_set_issuer(_offer, _txn, _issuer)
	char *_offer;
	unsigned int _txn;
	char *_issuer;
	CODE:
	{
	  bom::offer offer(_offer);
	  txn_t *txn = (txn_t *)_txn;
	  bom::acct issuer(_issuer);
	  RETVAL = offer.set_issuer(txn, issuer);
	}
	OUTPUT:
	RETVAL

int
_offer_del_issuer(_offer, _txn)
	char *_offer;
	unsigned int _txn;
	CODE:
	{
	  bom::offer offer(_offer);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = offer.del_issuer(txn);
	}
	OUTPUT:
	RETVAL
	
int
_offer_get_ask_value(_offer, _txn, _vl)
	char *_offer;
	unsigned int _txn;
	char *_vl;
	CODE:
	{
	  bom::offer offer(_offer);
	  txn_t *txn = (txn_t *)_txn;
	  bom::value vl;
	  RETVAL = offer.get_ask_value(txn, &vl);
	  if (!RETVAL)
	    value_to_string(vl, &_vl);
	}
	OUTPUT:
	RETVAL
	_vl

int
_offer_set_ask_value(_offer, _txn, _vl)
	char *_offer;
	unsigned int _txn;
	char *_vl;
	CODE:
	{
	  bom::offer offer(_offer);
	  txn_t *txn = (txn_t *)_txn;
	  bom::value vl;
	  string_to_value(_vl, &vl);
	  RETVAL = offer.set_ask_value(txn, vl);
	}
	OUTPUT:
	RETVAL

int
_offer_del_ask_value(_offer, _txn)
	char *_offer;
	unsigned int _txn;
	CODE:
	{
	  bom::offer offer(_offer);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = offer.del_ask_value(txn);
	}
	OUTPUT:
	RETVAL

int
_offer_get_bid_value(_offer, _txn, _vl)
	char *_offer;
	unsigned int _txn;
	char *_vl;
	CODE:
	{
	  bom::offer offer(_offer);
	  txn_t *txn = (txn_t *)_txn;
	  bom::value vl;
	  RETVAL = offer.get_bid_value(txn, &vl);
	  if (!RETVAL)
	    value_to_string(vl, &_vl);
	}
	OUTPUT:
	RETVAL
	_vl

int
_offer_set_bid_value(_offer, _txn, _vl)
	char *_offer;
	unsigned int _txn;
	char *_vl;
	CODE:
	{
	  bom::offer offer(_offer);
	  txn_t *txn = (txn_t *)_txn;
	  bom::value vl;
	  string_to_value(_vl, &vl);
	  RETVAL = offer.set_bid_value(txn, vl);
	}
	OUTPUT:
	RETVAL

int
_offer_del_bid_value(_offer, _txn)
	char *_offer;
	unsigned int _txn;
	CODE:
	{
	  bom::offer offer(_offer);
	  txn_t *txn = (txn_t *)_txn;
	  RETVAL = offer.del_bid_value(txn);
	}
	OUTPUT:
	RETVAL
