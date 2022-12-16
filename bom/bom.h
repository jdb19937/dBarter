// {HEADER}

#ifndef _BOM_H
#define _BOM_H

extern "C" {
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <features.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/file.h>
#include <sys/fcntl.h>
#include <netdb.h>
#include <sched.h>

#include <gmp.h>
#include <db.h>
}

#include <dmath.h>

#undef FALSE
#define FALSE		0

#undef TRUE
#define TRUE		1

extern int debugf(char *fmt, ...);
extern int warningf(char *fmt, ...);
extern int errorf(char *fmt, ...);
extern int sprintf_alloc(char **s, char *fmt, ...);

typedef DB_TXN txn_t;
#define txn_new(txn)		txn_begin(db_txnmgr, NULL, txn)

#define OBJECT_ATTR_ALL		0xffffffff
  
enum {
  DENOM_ATTR_PROMISE =		(1 << 1),
  DENOM_ATTR_ISSUER =		(1 << 2)
};

enum {
  ACCT_FLAG_SUPER	= (1 << 0),
  ACCT_FLAG_ISSUER	= (1 << 1),
  ACCT_FLAG_TXN		= (1 << 2)
};

#ifndef _ACCT_CC
extern char *acct_flag_string[];
#endif

enum {
  ACCT_ATTR_PUBLIC =		(1 << 0),
  ACCT_ATTR_SECRET =		(1 << 1),
  ACCT_ATTR_MODULUS =		(1 << 2),
  ACCT_ATTR_PROMISE =		(1 << 3),
  ACCT_ATTR_VALUES =		(1 << 4),
  ACCT_ATTR_NOTES =		(1 << 5),
  ACCT_ATTR_EVENTS =		(1 << 6),
  ACCT_ATTR_FLAGS =		(1 << 7),
  ACCT_ATTR_OFFERS =		(1 << 8),
};

enum {
  OFFER_ATTR_ISSUER =		(1 << 0),
  OFFER_ATTR_BID_VALUE =	(1 << 1),
  OFFER_ATTR_ASK_VALUE =	(1 << 2)
};

enum {
  NOTE_ATTR_ISSUER =		(1 << 0),
  NOTE_ATTR_CHALLENGE =		(1 << 1),
  NOTE_ATTR_VALUES =		(1 << 2)
};

enum {
  TID_ATTR_ACCT =		(1 << 0),
  TID_ATTR_COMMAND =		(1 << 1),
};
  
enum {
  EVENT_TYPE_CREDIT,
  EVENT_TYPE_DEBIT,
  EVENT_TYPE_ACCT_CREATE,
  EVENT_TYPE_ACCT_DESTROY,
  EVENT_TYPE_NOTE_CREATE,
  EVENT_TYPE_NOTE_DESTROY,
  EVENT_TYPE_DENOM_CREATE,
  EVENT_TYPE_DENOM_DESTROY,
  EVENT_TYPE_OFFER_CREATE,
  EVENT_TYPE_OFFER_DESTROY,
  EVENT_TYPE_EVENT_SENT,
  EVENT_TYPE_EVENT_RECEIVED,
};

#ifndef _EVENT_CC
extern char *event_name[];
#endif

enum {
  EVENT_ATTR_TYPE =		(1 << 0),
  EVENT_ATTR_DATA =		(1 << 1),
  EVENT_ATTR_ACCTS =		(1 << 2)
};
  
struct bom {
  struct object;
  struct acct;
  struct note;
  struct offer;
  struct denom;
  struct value;
  struct event;
  struct tid;
  
  struct object {
    char *id;
    
    object();
    object(char *);
    object(const object &);
    ~object();
    
    int set_id(char *);
  };
  
  typedef u_int32_t denom_attr;
  
  struct denom : object {
    char *_promise;
    acct *_issuer;
  
    denom *next;
  
    denom();
    denom(char *_id);
    denom(const denom& v);
    ~denom();
  
    int exists(txn_t *, char *) const;
    int destroy(txn_t *) const;
    
    int get(txn_t *, u_int32_t);
    int set(txn_t *, u_int32_t);
    
    int get_promise(txn_t *, char **) const;
    int set_promise(txn_t *, char *) const;
    int del_promise(txn_t *) const;
    
    int get_issuer(txn_t *, acct *) const;
    int set_issuer(txn_t *, const acct &) const;
    int del_issuer(txn_t *) const;
    
    friend int get_denom_list(txn_t *, denom **);
    friend int search_denom_list(txn_t *, char *, denom **);

    friend int denom_list_to_string(denom *, char **);
    friend int string_to_denom_list(char *, denom **);
  };
  
  struct value {
    bom::denom denom;
    dmath::bq quant;
    value *next;
  
    value();
    value(const bom::denom &, const dmath::bq &);
    value(const value& v);
    ~value();
    
    value &operator=(const value &);
  
    int set_denom(const bom::denom &);
    int set_quant(const dmath::bq &);
    
    friend int string_to_value(char *, value *);
    friend int value_to_string(const value &, char **);
    friend int portfolio_to_string(value *, char **);
    friend int string_to_portfolio(char *, value **);
  };
  
  typedef u_int32_t acct_flag;
  typedef u_int32_t acct_attr;
  
  struct acct : object {
    dmath::bz *_public;
    dmath::bz *_secret;
    dmath::bz *_modulus;
    char *_promise;
    value *_values;
    note *_notes;
    offer *_offers;
    event *_events;
    acct_flag _flags;
  
    acct *next;
  
    acct();
    acct(char *);
    acct(const acct &);
    ~acct();
    
    int exists(txn_t *, char *);
    int destroy(txn_t *);
  
    int get(txn_t *, u_int32_t);
    int set(txn_t *, u_int32_t);
  
    int get_modulus(txn_t *, dmath::bz *) const;
    int set_modulus(txn_t *, const dmath::bz &) const;
    int del_modulus(txn_t *) const;
  
    int get_public(txn_t *, dmath::bz *) const;
    int set_public(txn_t *, const dmath::bz &) const;
    int del_public(txn_t *) const;
  
    int get_secret(txn_t *, dmath::bz *) const;
    int set_secret(txn_t *, const dmath::bz &) const;
    int del_secret(txn_t *) const;
  
    int get_promise(txn_t *, char **) const;
    int set_promise(txn_t *, char *) const;
    int del_promise(txn_t *) const;
    
    int create_challenge(txn_t *, dmath::bz *, char *);
    int verify_reponse(txn_t *, const dmath::bz &, const dmath::bz &, char *);
    
    int get_value(txn_t *, value *val);
    int set_value(txn_t *, const value &val);
    int del_value(txn_t *, const value &val);
    int get_value_first(DBC *, value *, char *);
    int get_value_next(DBC *, value *, char *);

    int get_portfolio(txn_t *, value **);
    int set_portfolio(txn_t *, value *);
    int del_portfolio(txn_t *);
    
    int get_note_list(txn_t *, note **);
    int del_note_list(txn_t *);
  
    int get_offer_list(txn_t *, offer **);
    int del_offer_list(txn_t *);
  
    int get_denom_list(txn_t *, denom **);
    int get_event_list(txn_t *, event **);
  
    int credit(txn_t *, const value &, char *);
    int debit(txn_t *, const value &, char *);
    int debit_forced(txn_t *, const value &);
  
    int get_flags(txn_t *, acct_flag *) const;
    int set_flags(txn_t *, acct_flag) const;
    int del_flags(txn_t *) const;
    
    friend int get_acct_list(txn_t *, acct **);

    friend int acct_list_to_string(acct *, char **);
    friend int string_to_acct_list(char *, acct **);
    friend int acct_flags_to_string(acct_flag, char **);
    friend int string_to_acct_flags(char *, acct_flag *);
  };
  
  typedef u_int32_t offer_attr;
  
  struct offer : object {
    acct *issuer;
    value *bid_value;
    value *ask_value;
  
    offer *next;
  
    offer();
    offer(char *);
    offer(const offer &);
    ~offer();
    
    int gen_id();
    
    int exists(txn_t *txn, char *exists);
    int destroy(txn_t *txn);
  
    int get(txn_t *, u_int32_t);
    int set(txn_t *, u_int32_t);
    
    int get_issuer(txn_t *txn, acct *issuer);
    int set_issuer(txn_t *txn, const acct &issuer);
    int del_issuer(txn_t *txn);
  
    int get_bid_value(txn_t *txn, value *value);
    int set_bid_value(txn_t *txn, const value &value);
    int del_bid_value(txn_t *txn);
  
    int get_ask_value(txn_t *txn, value *value);
    int set_ask_value(txn_t *txn, const value &value);
    int del_ask_value(txn_t *txn);
    
    int set_index(txn_t *txn, const denom &, const denom &,
     const dmath::bq &ratio);
    int set_index(txn_t *txn, const value &, const value &);
    int del_index(txn_t *txn, const denom &, const denom &);
  
    friend int find_best_offer(txn_t *txn,
     const denom &ask, const denom &bid,
     offer *offer, dmath::bq *ratio, char *exists);
    friend int find_all_offers(txn_t *txn,
     const denom &ask, const denom &bid,
     offer **offer_list);

    friend int offer_list_to_string(offer *, char **);
    friend int string_to_offer_list(char *, offer **);
  };
  
  typedef u_int32_t note_attr;
  
  struct note : object {
    acct *_issuer;
    dmath::bz *_challenge;
    value *_values;
  
    note *next;
  
    note();
    note(char *);
    note(const note &);
    ~note();
    
    int gen_id();
  
    int exists(txn_t *, char *);
    int destroy(txn_t *);
  
    int get(txn_t *, u_int32_t);
    int set(txn_t *, u_int32_t);
  
    int get_issuer(txn_t *, acct *);
    int set_issuer(txn_t *, const acct &);
    int del_issuer(txn_t *);
  
    int get_challenge(txn_t *, dmath::bz *);
    int set_challenge(txn_t *, const dmath::bz &);
    int del_challenge(txn_t *);
  
    int get_value(txn_t *, value *);
    int set_value(txn_t *, const value &);
    int del_value(txn_t *, const value &);
    int get_value_first(DBC *, value *, char *);
    int get_value_next(DBC *, value *, char *);

    int get_portfolio(txn_t *, value **);
    int set_portfolio(txn_t *, value *);
    int del_portfolio(txn_t *);
  
    int create_challenge(txn_t *, dmath::bz *, char *);
    int verify_reponse(txn_t *, const dmath::bz &, const dmath::bz &, char *);

    friend int note_list_to_string(note *, char **);
    friend int string_to_note_list(char *, note **);
  };
  
  typedef u_int32_t tid_attr;
  
  struct tid : object {
    bom::acct *acct;
    char *command;
  
    tid *next;
  
    tid();
    tid(char *_id);
    tid(const tid &);
    ~tid();
  
    int gen_id();
  
    int exists(txn_t *, char *);
    int destroy(txn_t *);
  
    int get(txn_t *, u_int32_t);
    int set(txn_t *, u_int32_t);
    
    int get_command(txn_t *, char **);
    int set_command(txn_t *, char *);
    int del_command(txn_t *);
    
    int get_acct(txn_t *, bom::acct *);
    int set_acct(txn_t *, const bom::acct &);
    int del_acct(txn_t *);
  };
  
  typedef u_int32_t event_id;
  typedef u_int32_t event_type;
  typedef u_int32_t event_attr;
  
  struct event {
    event_id id;
  
    event_type _type;
    char *_data;
    acct *_accts;
  
    event *next;
    
    event();
    event(event_id);
    event(const event &);
    ~event();
  
    int set_id(event_id);
    int gen_id(txn_t *);
  
    int exists(txn_t *, char *);
    int destroy(txn_t *);
  
    int get(txn_t *, u_int32_t);
    int set(txn_t *, u_int32_t);
  
    int get_type(txn_t *txn, event_type *type);
    int set_type(txn_t *txn, event_type type);
    int del_type(txn_t *txn);
  
    int get_data(txn_t *txn, char **data);
    int set_data(txn_t *txn, char *data);
    int del_data(txn_t *txn);
  
    int get_acct_list(txn_t *txn, acct **) const;
    int set_acct_list(txn_t *txn, const acct *) const;
    int del_acct_list(txn_t *txn) const;
    
    friend int get_last_event(txn_t *, event *);
  
    friend int event_new_credit(txn_t *txn, event *,
     const acct &acct, const value &value);
    friend int event_new_debit(txn_t *txn, event *,
     const acct &acct, const value &value);
    friend int event_new_note_create(txn_t *, event *, const note &);
    friend int event_new_note_destroy(txn_t *, event *, const note &);
    friend int event_new_acct_create(txn_t *, event *, const acct &);
    friend int event_new_acct_destroy(txn_t *, event *, const acct &);
    friend int event_new_denom_create(txn_t *, event *, const denom &);
    friend int event_new_denom_destroy(txn_t *, event *, const denom &);
    friend int event_new_offer_create(txn_t *, event *, const offer &);
    friend int event_new_offer_destroy(txn_t *, event *, const offer &);
    friend int event_new_event_sent(txn_t *, event *,
     const acct &, const acct &, char *);
    friend int event_new_event_received(txn_t *, event *,
     const acct &, const acct &, char *);
    
    friend int event_to_string(const event &, char **);
    friend int string_to_event(char *, event *);

    friend int event_list_to_string(event *, char **);
    friend int string_to_event_list(char *, event **);
  };
};
  
typedef enum {
  DB_CONFIG,
  DB_ACCT_PUBLIC,
  DB_ACCT_SECRET,
  DB_NOTE_CHALLENGE,
  DB_NOTE_ISSUER,
  DB_ACCT_NOTE,
  DB_ACCT_MODULUS,
  DB_NOTE_VALUE,
  DB_ACCT_VALUE,
  DB_DENOM_TYPE,
  DB_DENOM_PROMISE,
  DB_DENOM_ISSUER,
  DB_ISSUER_DENOM,
  DB_ACCT_FLAGS,
  DB_EVENT_TYPE,
  DB_EVENT_DATA,
  DB_EVENT_ACCT,
  DB_ACCT_EVENT,
  DB_TID_ACCT,
  DB_TID_COMMAND,
  DB_ACCT_PROMISE,
  
  DB_OFFER_INDEX,
  DB_OFFER_BID_VALUE,
  DB_OFFER_ASK_VALUE,
  DB_OFFER_ISSUER,
  DB_ACCT_OFFER,
} db_file_t;

struct db_info_t {
  char *name;
  DB* db;
  DBTYPE type;
  DB_INFO *info;
};

#ifndef _DB_CC
extern DB_ENV db_env;
extern DB_TXNMGR* db_txnmgr;
extern DB_LOCKTAB* db_locktab;
extern db_info_t db_info[];
#endif

void db_init(bool);
void db_exit();
DB* db_insure_open(int dn);
void db_close(int dn);

int bom_db_get(char *, DB_TXN *, char *, char **);
int bom_db_put(char *, DB_TXN *, char *, char *);
int bom_db_del(char *, DB_TXN *, char *);

int dbt_init(DBT *d);
int dbt_clear(DBT *d);
int string_to_dbt(char *s, DBT *d);
int dbt_to_string(const DBT &d, char **s);
int int32_to_dbt(u_int32_t, DBT *d);
int dbt_to_int32(const DBT &d, u_int32_t *);

extern void bom_db_init(char *, int threaded = 0);
extern void bom_db_exit();
extern int bom_jump_set(char *, void *);

#ifndef _ENVIRON_CC
extern char *bom_db_directory;
extern void *(*bom_os_malloc)(size_t);
extern void (*bom_os_free)(void *);
extern void *(*bom_os_realloc)(void *, size_t);
extern char *(*bom_os_strdup)(char *);
#endif

extern void *operator new(size_t);
extern void operator delete(void *);

#endif
