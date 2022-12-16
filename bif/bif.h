// {HEADER}

#ifndef _BIF_H
#define _BIF_H

#include <bom.h>
#include <dthread.h>

typedef enum {
  CODE_WELCOME =		100,

  CODE_ERR_USAGE =		333,
  CODE_ERR_UNKCOMMAND =		334,
  CODE_ERR_LINETOOLONG =	338,
  CODE_ERR_NOAUTH =		339,
  CODE_ERR_DB =			340,
  CODE_ERR_INVALID =		341,
  CODE_ERR_INSUFFICIENT =	343,
  CODE_ERR_NET =		352,
  CODE_ERR_EXISTS =		358,
  CODE_ERR_NOTFOUND =		359,
  CODE_ERR_ABORTED =		360,

  CODE_EVENT =			400,
  CODE_SUCCESS =		500,

  CODE_GOODBYE =		923
} response_code_t; 

#define STATUS_NULL		0
#define STATUS_AUTHENTICATED	1

struct bif {
  struct site;
  struct message;
  struct client;
  struct shell;
  struct server;
  struct socket;
  struct trader;
  struct market;
  struct session;
  
  struct site {
    char *host;
    unsigned short port;
  
    site *next;
  
    site();
    site(char *);
    site(char *, unsigned short);
    site(const site &);
    ~site();
  
    site &operator=(const site &);
    
    int set_host(char *_host);
    int set_port(unsigned short _port);
    
    friend char operator==(const site &, const site &);
    friend char operator!=(const site &, const site &);
  
    friend int string_to_remote(char *, char **, site *);
    friend int remote_to_string(char *, const site &, char **);
    friend int site_to_string(const site &, char **);
    friend int string_to_site(char *, site *);
  };
  
  typedef int (shell::*shell_command)(int, char **);
  
  struct shell_index {
    char *command;
    shell_command function;
  };
  
  struct socket {
    int s;
    struct sockaddr_in addr;
    struct sockaddr_in to_addr;
  
    socket();
    ~socket();
    
    int read(void *buf, size_t count);
    int write(void *buf, size_t count);
    int close();
    
    void writef(char *fmt, ...);
  
    friend int find_host(char *, int, struct sockaddr_in *);
    friend int find_host(int, struct sockaddr_in *);
    friend void block_socket(int, char);
  };
  
  struct shell : socket, dthread::thread, dthread::mutex {
    server *sv;
    time_t t0, active_t0;
    DB_TXN *parent;
    shell *prev, *next;
  
    bif::session *session;
    bom::acct acct;
    dmath::bz challenge;
  
    char status;
  
    shell(int, struct sockaddr_in *, server *);
    ~shell();
  
    void link();
    void unlink();
    int process();
  
    int cmd_login(int, char **);
    int cmd_respond(int, char **);
  
    int cmd_exit(int, char **);
    int cmd_shutdown(int, char **);
  
    int cmd_acct_create(int, char **);
    int cmd_acct_destroy(int, char **);
    int cmd_acct_set(int, char **);
    int cmd_acct_read(int, char **);
  
    int cmd_note_read(int, char **);
    int cmd_note_list(int, char **);
    int cmd_note_deposit(int, char **);
    int cmd_note_withdraw(int, char **);
  
    int cmd_denom_create(int, char **);
    int cmd_denom_destroy(int, char **);
    int cmd_denom_read(int, char **);
    int cmd_denom_set(int, char **);
    int cmd_denom_list(int, char **);
    int cmd_denom_list_all(int, char **);
    int cmd_denom_search(int, char **);
  
    int cmd_offer_read(int, char **);
    int cmd_offer_list(int, char **);
    int cmd_offer_list_by_denom(int, char **);
    int cmd_offer_create(int, char **);
    int cmd_offer_destroy(int, char **);
  
    int cmd_event_send(int, char **);
    int cmd_event_list(int, char **);
    int cmd_event_read(int, char **);
    int cmd_event_last(int, char **);
  
    int cmd_balance(int, char **);
    int cmd_credit(int, char **);
    int cmd_debit(int, char **);
  
    int cmd_connect(int, char **);
    int cmd_disconnect(int, char **);
    int cmd_reconnect(int, char **);
  
    int cmd_help(int, char **);
    
    int cmd_txn_create(int, char **);
    int cmd_txn_commit(int, char **);
    int cmd_txn_abort(int, char **);
    int cmd_txn_read(int, char **);
  };
  
  struct server : socket, dthread::mutex {
    bif::site site;
    bif::shell *client_list;
    bom::event_id event_id;
    bif::market *market;
    
    dthread::mutex event_mutex;
    dthread::condition event_condition;
    dthread::thread event_thread;
  
    server();
    ~server();
  
    int open(const bif::site &);
    int open(char *, int);
    int open(int);
    int accept();
    
    void lock_acct(const bom::acct &acct);
    void unlock_acct(const bom::acct &acct);
    void write_acct(const bom::acct &, const message &);
    void writef_acct(const bom::acct &, char *fmt, ...);
    
    void event_update();
    void process_events();
  };
  
  #define MESSAGE_WELCOME	1
  #define MESSAGE_ERROR	3
  #define MESSAGE_ASYNC	4
  #define MESSAGE_SUCCESS	5
  #define MESSAGE_GOODBYE	9
  
  struct message {
    int code;
    char *desc;
    char *data;
    message *next;
    
    message();
    ~message();
    
    int set_desc(char *);
    int set_data(char *);
  
    int type() {
      return (code / 100);
    }
  };
  
  typedef int (*event_handler_f)(client *, const message &);
  
  struct client : dthread::thread, dthread::mutex, socket {
    bif::site *site;
    bif::message *messages, *messages_tail;
  
    client *next;
    
    client();
    ~client();
    
    int connect(const bif::site &);
    int close();
  
    int read_one_message(bif::message *);
    int read_message(bif::message *);
    
    int cmd_login(const bom::acct &, dmath::bz *);
    int cmd_respond(const dmath::bz &);
  
    int cmd_event_send(const bom::acct &, char *, bom::event *);
  
    int cmd_note_read(bom::note *);
    int cmd_note_withdraw(bom::value *, bom::note *);
    int cmd_note_deposit(const bom::note &, const dmath::bz &, bom::value **);
  
    int cmd_txn_create(bom::tid *);
    int cmd_txn_abort();
    int cmd_txn_commit();
  };
  
  struct session : bom::object {
    bif::session *prev, *next;
    bif::shell *shell;
    bif::client client;
    time_t t0;
  
    session(bif::shell *);
    ~session();
  
    void link();
    void unlink();
  
    int gen_id();
  
    friend session *find_session(char *id);
    friend void expire_sessions();
  };
  
  struct trader : dthread::thread {
    bif::market *market;
    bom::denom denom[2];
  
    trader *prev, *next;
  
    trader(bif::market *_market, const bom::denom &d0, const bom::denom &d1);
    ~trader();
    
    void link();
    void unlink();
  
    int execute(DB_TXN *txn, char *);
    int process();
  };
  
  struct market : dthread::mutex {
    server *sv;
    trader *trader_list;
    bom::denom *denom_list;
  
    market(server *);
    ~market();
    
    int signal(const bom::denom &d0, const bom::denom &d1);
  };
};

#ifndef _SESSION_CC
extern bif::session *session_list;
#endif

extern int encode_string(int, char **, char **);
extern int decode_string(char *, int *, char ***);
extern char *hash_extract(char *, char *, char **);

#ifndef _ENVIRON_CC
extern void (*bif_shutdown)();
extern int bif_jump_set(char *, void *);
#endif

#define DEFAULT_PORT		1134
  
#endif
