// {HEADER}

#define _SHELL_CC
#include "bom.h"
#include "bif.h"

struct bif::shell_index command_table[] = {
  { "help", &bif::shell::cmd_help },

  { "exit", &bif::shell::cmd_exit },
  { "shutdown", &bif::shell::cmd_shutdown },

  { "login", &bif::shell::cmd_login },
  { "respond", &bif::shell::cmd_respond },
  { "connect", &bif::shell::cmd_connect },
  { "reconnect", &bif::shell::cmd_reconnect },
  { "disconnect", &bif::shell::cmd_disconnect },

  { "txn_create", &bif::shell::cmd_txn_create },
  { "txn_commit", &bif::shell::cmd_txn_commit },
  { "txn_abort", &bif::shell::cmd_txn_abort },
  { "txn_read", &bif::shell::cmd_txn_read },

  { "event_read", &bif::shell::cmd_event_read },
  { "event_list", &bif::shell::cmd_event_list },
  { "event_last", &bif::shell::cmd_event_last },
  { "event_send", &bif::shell::cmd_event_send },

  { "acct_create", &bif::shell::cmd_acct_create },
  { "acct_read", &bif::shell::cmd_acct_read },
  { "acct_set", &bif::shell::cmd_acct_set },
  { "acct_destroy", &bif::shell::cmd_acct_destroy },

  { "denom_create", &bif::shell::cmd_denom_create }, 
  { "denom_read", &bif::shell::cmd_denom_read },
  { "denom_set", &bif::shell::cmd_denom_set },
  { "denom_destroy", &bif::shell::cmd_denom_destroy },
  { "denom_list_all", &bif::shell::cmd_denom_list_all },
  { "denom_list", &bif::shell::cmd_denom_list },
  { "denom_search", &bif::shell::cmd_denom_search },

  { "note_read", &bif::shell::cmd_note_read },
  { "note_list", &bif::shell::cmd_note_list },
  { "note_withdraw", &bif::shell::cmd_note_withdraw },
  { "note_deposit", &bif::shell::cmd_note_deposit },

  { "offer_read", &bif::shell::cmd_offer_read },
  { "offer_list", &bif::shell::cmd_offer_list },
  { "offer_list_by_denom", &bif::shell::cmd_offer_list_by_denom },
  { "offer_create", &bif::shell::cmd_offer_create },
  { "offer_destroy", &bif::shell::cmd_offer_destroy },

  { "balance", &bif::shell::cmd_balance },
  { "credit", &bif::shell::cmd_credit },
  { "debit", &bif::shell::cmd_debit },

  { "", 0 }
};

bif::shell_command command_lookup(char *command) {
  for (int i = 0; command_table[i].function; i++)
    if (!strcasecmp(command_table[i].command, command))
      return command_table[i].function;
  return NULL;
}

#define LINE_LENGTH_BEGIN	64
#define LINE_LENGTH_MAX		65536
#define MAX_ARGUMENTS		16

enum {
  READ_LINE_SUCCESS =		0,
  READ_LINE_TOO_LONG =		-1,
  READ_LINE_ERROR =		-2
};

int shell_read_line(bif::shell *s, char **line) {
  char c;
  int n;
  int length = 0, alength;

  *line = (char *)bom_os_malloc(alength = LINE_LENGTH_BEGIN);

  while ((n = s->read(&c, 1)) == 1) {
    if (length >= LINE_LENGTH_MAX)
      break;
    if (length >= alength)
      *line = (char *)bom_os_realloc(*line, alength *= 2);
    if (c == '\n' && length > 0 && *(*line + length - 1) == '\r')
      break;
    *(*line + length++) = c;
  }
  
  if (length > 0 && *(*line + length - 1) == '\r')
    *(*line + length - 1) = '\0';
  else
    *(*line + length) = '\0';

  if (n != 1) {
    bom_os_free(*line);
    *line = NULL;
    return READ_LINE_ERROR;
  }

  if (length == LINE_LENGTH_MAX) {
    char lastc = 0;

    while ((n = s->read(&c, 1)) == 1) {
      if (c == '\n' && lastc == '\r')
        break;
      lastc = c;
    }

    bom_os_free(*line);
    *line = NULL;
    return READ_LINE_TOO_LONG;
  }

  return READ_LINE_SUCCESS;
}

enum {
  PARSE_LINE_SUCCESS =		0,
  PARSE_LINE_TOO_MANY =		-1
};

int shell_parse_line(char *line, int *argc, char ***argv) {
  return decode_string(line, argc, argv);
}

int bif::shell::process() {
  char *line;
  int argc;
  char **argv;
  char *slocal_site;
  bif::shell_command function;
  int result;
  
  site_to_string(sv->site, &slocal_site);

  lock();
  writef("%03d barterd " VERSION " /jdb\n%s\n.\n", CODE_WELCOME, slocal_site);
  unlock();
  
  bom_os_free(slocal_site);

  while (1) {
    result = shell_read_line(this, &line);
    if (result == READ_LINE_ERROR)
      return result;
    if (result == READ_LINE_TOO_LONG) {
      lock();
      writef("%03d line too long\n.\n", CODE_ERR_INVALID);
      unlock();
      continue;
    }
     
    if (result = shell_parse_line(line, &argc, &argv)) {
      lock();
      writef("%03d parse error\n.\n", CODE_ERR_INVALID);
      unlock();
      bom_os_free(line);
      continue;
    }

    if (argc == 0 || !(function = command_lookup(argv[0]))) {
      lock();
      writef("%03d unknown command\n.\n", CODE_ERR_INVALID);
      unlock();
      bom_os_free(line);
      for (int i = 0; i < argc; i++)
        bom_os_free(argv[i]);
      bom_os_free(argv);
      continue;
    }

    active_t0 = time(0);

    lock();

    if (session) {
      if (function == &bif::shell::cmd_exit)
        result = (this->*function)(argc, argv);
      if (function == &bif::shell::cmd_disconnect)
        result = (this->*function)(argc, argv);
      else if (function == &bif::shell::cmd_connect)
	writef("%03d already connected\n.\n", CODE_ERR_INVALID);
      else if (function == &bif::shell::cmd_reconnect)
	writef("%03d already connected\n.\n", CODE_ERR_INVALID);
      else {
        bif::message m;

        session->client.writef("%s\r\n", line);

        if (int result = session->client.read_message(&m)) {
	  bif::session *_session = session;
	  session = NULL;
	  delete _session;

	  writef("%03d goodbye: error %d\n.\n", CODE_GOODBYE, result);
	} else {
          for (bif::message *mm = session->client.messages; mm; mm = mm->next) 
            writef("%s\n%s.\n", mm->desc, mm->data);
          writef("%s\n%s.\n", m.desc, m.data);
	}
      }
    } else {
      result = (this->*function)(argc, argv);
    }

    unlock();

    sv->event_update();

    bom_os_free(line);
    for (int i = 0; i < argc; i++)
      bom_os_free(argv[i]);
    bom_os_free(argv);
  }
}

void *_shell_thread(void *_c) {
  bif::shell *c = (bif::shell *)_c;
  int retval;

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  
  c->process();

  delete c;

  thread_exit();
}

bif::shell::shell(int _s, struct sockaddr_in* from, bif::server *_sv) {
  s = _s;
  sv = _sv;
  parent = NULL;
  session = NULL;
  status = STATUS_NULL;
  memcpy(&to_addr, from, sizeof(struct sockaddr_in));

  link();
  t0 = time(0);
  active_t0 = time(0);

  spawn(_shell_thread, this);
  detach();
}

bif::shell::~shell() {
  if (parent) {
    txn_abort(parent);
    parent = NULL;
  }
  
  close();
  unlink();
}

void bif::shell::link() {
  bif::shell **head;
 
  sv->lock();
  head = &sv->client_list; 
  prev = NULL;
  next = *head;
  if (*head)
    (*head)->prev = this;
  *head = this;
  sv->unlock();
}

void bif::shell::unlink() {
  bif::shell **head;
 
  sv->lock();
  head = &sv->client_list; 
  if (prev)
    prev->next = next;
  if (next)
    next->prev = prev;
  if (this == *head)
    *head = (bif::shell *)next;
  sv->unlock();
}

#define begin_transaction(txn, arg1) { \
  if (parent) \
    txn = parent; \
  else { \
    int result = txn_new(&txn); \
    if (result) { \
      arg1; \
      writef("%03d database error: %d\n.\n", CODE_ERR_DB, result); \
      return 0; \
    } \
  } \
}

#define abort_transaction(txn) { \
  if (!parent) \
    txn_abort(txn); \
}

#define commit_transaction(txn, arg1, arg2) { \
  if (!parent) { \
    int result = txn_commit(txn); \
    if (result == EAGAIN) { \
      txn_abort(txn); \
      arg1; \
      goto again; \
    } else if (result) { \
      txn_abort(txn); \
      arg1; \
      arg2; \
      writef("%03d database error: %d\n.\n", CODE_ERR_DB, result); \
      return 0; \
    } \
  } \
}

#define prepare_transaction(txn, arg1, arg2) { \
  if (!parent) { \
    int result = txn_prepare(txn); \
    if (result == EAGAIN) { \
      txn_abort(txn); \
      arg1; \
      goto again; \
    } else if (result) { \
      txn_abort(txn); \
      arg1; \
      arg2; \
      writef("%03d database error: %d\n.\n", CODE_ERR_DB, result); \
      return 0; \
    } \
  } \
}


#define error_check(txn, result, arg1, arg2) { \
  if (result == EAGAIN) { \
    txn_abort(txn); \
    arg1; \
    if (parent) { \
      parent = NULL; \
      arg2; \
      writef("%03d transaction aborted\n.\n", CODE_ERR_ABORTED); \
      return 0; \
    } \
    goto again; \
  } else if (result) { \
    txn_abort(txn); \
    parent = NULL; \
    arg1; \
    arg2; \
    if (parent) \
      writef("%03d database error: %d, transaction aborted\n.\n", \
       CODE_ERR_ABORTED, result); \
    else \
      writef("%03d database error: %d\n.\n", CODE_ERR_DB, result); \
    return 0; \
  } \
}

#define new_event(txn, result, type, object) { \
  bom::event event; \
  result = event_new_ ## type (txn, &event, object); \
}

int bif::shell::cmd_denom_create(int argc, char **argv) {
  txn_t* txn;
  int result;
  bom::denom denom;

  if (argc != 2) {
    writef("%03d usage: denom_create denom\n.\n",
     CODE_ERR_USAGE);
    return 0;
  }
  
  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }

  denom.set_id(argv[1]);

again:
  begin_transaction(txn, 0);

  {
    char exists;
    result = denom.exists(txn, &exists);
    error_check(txn, result, 0, 0);
  
    if (exists) {
      abort_transaction(txn);
      writef("%03d denom already exists\n.\n", CODE_ERR_EXISTS);
      return 0;
    }
  }

  {
    bom::acct_flag flags;

    result = acct.get_flags(txn, &flags);
    error_check(txn, result, 0, 0);
    
    if (!(flags & ACCT_FLAG_ISSUER) && !(flags & ACCT_FLAG_SUPER)) {
      abort_transaction(txn);
      writef("%03d no issuer permissions\n.\n", CODE_ERR_NOAUTH);
      return 0;
    }
  }

  result = denom.set_issuer(txn, acct);
  error_check(txn, result, 0, 0);

  result = denom.set_promise(txn, "");
  error_check(txn, result, 0, 0);
  
  denom._issuer = &acct;
  new_event(txn, result, denom_create, denom);
  denom._issuer = NULL;
  error_check(txn, result, 0, 0);
  
  commit_transaction(txn, 0, 0);

  writef("%03d success\n.\n", CODE_SUCCESS);
  return 0;
}

int bif::shell::cmd_denom_set(int argc, char **argv) {
  txn_t* txn;
  int result;
  bom::denom denom;
  char *attribute, *value;
  bom::acct_flag flags;
  bom::acct issuer;

  enum {
    _denom_attr_issuer,
    _denom_attr_promise,
  } n_attribute;

  if (argc != 4) {
    writef("%03d usage: denom_set denom attribute value\n.\n",
     CODE_ERR_USAGE);
    return 0;
  }
  
  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }

  denom.set_id(argv[1]);
  attribute = argv[2];
  value = argv[3];
  
  if (!strcmp(attribute, "promise"))
    n_attribute = _denom_attr_promise;
  else if (!strcmp(attribute, "issuer"))
    n_attribute = _denom_attr_issuer;
  else {
    writef("%03d attribute must be in (promise, issuer)\n.\n",
     CODE_ERR_INVALID);
    return 0;
  }
  
again:
  begin_transaction(txn, 0);
  
  {
    char exists;

    result = denom.exists(txn, &exists);
    error_check(txn, result, 0, 0);
  
    if (!exists) {
      abort_transaction(txn);
      writef("%03d no such denom\n.\n", CODE_ERR_NOTFOUND);
      return 0;
    }
  }

  result = acct.get_flags(txn, &flags);
  error_check(txn, result, 0, 0);
  
  if (!(flags & ACCT_FLAG_SUPER)) {
    if (!(flags & ACCT_FLAG_ISSUER)) {
      abort_transaction(txn);
      writef("%03d no issuer permissions\n.\n", CODE_ERR_NOAUTH);
      return 0;
    }

    if (n_attribute == _denom_attr_issuer) {
      abort_transaction(txn);
      writef("%03d no superacct permissions\n.\n", CODE_ERR_NOAUTH);
      return 0;
    }

    result = denom.get_issuer(txn, &issuer);
    error_check(txn, result, 0, 0);
  
    if (strcmp(issuer.id, acct.id)) {
      abort_transaction(txn);
      writef("%03d no superacct permissions\n.\n", CODE_ERR_NOAUTH);
      return 0;
    }
  }
  
  switch (n_attribute) {
  case _denom_attr_promise:
    {
      result = denom.set_promise(txn, value);
      break;
    }
  case _denom_attr_issuer: 
    {
      bom::acct _issuer(value);
      result = denom.set_issuer(txn, _issuer);
      break;
    }
  }

  error_check(txn, result, 0, 0);

  commit_transaction(txn, 0, 0);

  writef("%03d success\n.\n", CODE_SUCCESS);
  return 0;
}

int bif::shell::cmd_acct_create(int argc, char **argv) {
  txn_t* txn;
  int result;
  bom::acct new_acct;
  dmath::bz kpublic, kmodulus;

  if (argc != 4) {
    writef("%03d usage: acct_create acctid public modulus\n.\n",
     CODE_ERR_USAGE);
    return 0;
  }

  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }

  new_acct.set_id(argv[1]);
  
  if (string_to_bz(argv[2], &kpublic) || kpublic < 1) {
    writef("%03d invalid input (public key)\n.\n", CODE_ERR_INVALID);
    return 0;
  }
  
  if (string_to_bz(argv[3], &kmodulus) || kmodulus < 2) {
    writef("%03d invalid input (modulus)\n.\n", CODE_ERR_INVALID);
    return 0;
  }
  
again:
  begin_transaction(txn, 0);
  
  {
    bom::acct_flag flags;
    
    result = acct.get_flags(txn, &flags);
    error_check(txn, result, 0, 0);
    
    if (!(flags & ACCT_FLAG_SUPER)) {
      abort_transaction(txn);
      writef("%03d permission denied\n.\n", CODE_ERR_NOAUTH);
      return 0;
    }
  }

  {
    char exists;
    result = new_acct.exists(txn, &exists);
    error_check(txn, result, 0, 0);
  
    if (exists) {
      abort_transaction(txn);
      writef("%03d acct already exists\n.\n", CODE_ERR_EXISTS);
      return 0;
    }
  }

  result = new_acct.set_flags(txn, (bom::acct_flag)0);
  error_check(txn, result, 0, 0);

  result = new_acct.set_public(txn, kpublic);
  error_check(txn, result, 0, 0);

  result = new_acct.set_modulus(txn, kmodulus);
  error_check(txn, result, 0, 0);

  result = new_acct.set_promise(txn, "");
  error_check(txn, result, 0, 0);

  new_event(txn, result, acct_create, new_acct);
  error_check(txn, result, 0, 0);

  commit_transaction(txn, 0, 0);

  writef("%03d success\n.\n", CODE_SUCCESS);
  return 0;
}

int bif::shell::cmd_acct_set(int argc, char **argv) {
  txn_t* txn;
  int result;
  bom::acct _acct;
  char *attribute, *value;
  bom::acct_flag flags;
  bom::acct issuer;
  bom::acct_attr n_attribute;

  if (argc != 4) {
    writef("%03d usage: acct_set acct attribute value\n.\n",
     CODE_ERR_USAGE);
    return 0;
  }
  
  _acct.set_id(argv[1]);
  attribute = argv[2];
  value = argv[3];
  
  if (!strcmp(attribute, "flags"))
    n_attribute = ACCT_ATTR_FLAGS;
  else if (!strcmp(attribute, "promise"))
    n_attribute = ACCT_ATTR_PROMISE;
  else if (!strcmp(attribute, "public"))
    n_attribute = ACCT_ATTR_PUBLIC;
  else if (!strcmp(attribute, "secret"))
    n_attribute = ACCT_ATTR_SECRET;
  else if (!strcmp(attribute, "modulus"))
    n_attribute = ACCT_ATTR_MODULUS;
  else {
    writef("%03d attribute must be in (flags, public, secret, modulus)\n.\n",
     CODE_ERR_INVALID);
    return 0;
  }
  
again:
  begin_transaction(txn, 0);
  
  {
    char exists;

    result = _acct.exists(txn, &exists);
    error_check(txn, result, 0, 0);
  
    if (!exists) {
      abort_transaction(txn);
      writef("%03d no such acct\n.\n", CODE_ERR_NOTFOUND);
      return 0;
    }
  }

  result = acct.get_flags(txn, &flags);
  error_check(txn, result, 0, 0);
  
  if (!(flags & ACCT_FLAG_SUPER)) {
    if (n_attribute == ACCT_ATTR_FLAGS) {
      abort_transaction(txn);
      writef("%03d no superacct permissions\n.\n", CODE_ERR_NOAUTH);
      return 0;
    }

    if (strcmp(acct.id, _acct.id)) {
      abort_transaction(txn);
      writef("%03d no superacct permissions\n.\n", CODE_ERR_NOAUTH);
      return 0;
    }
  }
  
  switch (n_attribute) {
  case ACCT_ATTR_PUBLIC:
    {
      dmath::bz kpublic;
      string_to_bz(value, &kpublic);
      result = _acct.set_public(txn, kpublic);
      break;
    }
  case ACCT_ATTR_SECRET:
    {
      dmath::bz ksecret;
      string_to_bz(value, &ksecret);
      result = _acct.set_secret(txn, ksecret);
      break;
    }
  case ACCT_ATTR_MODULUS:
    {
      dmath::bz kmodulus;
      string_to_bz(value, &kmodulus);
      result = _acct.set_modulus(txn, kmodulus);
      break;
    }
  case ACCT_ATTR_FLAGS:
    {
      char *sflags = value;
      bom::acct_flag _flags = (bom::acct_flag)0;
      
      if (int result = string_to_acct_flags(sflags, &_flags)) {
        abort_transaction(txn);
        writef("%03d invalid input\n.\n", CODE_ERR_INVALID);
	return 0;
      }

      result = _acct.set_flags(txn, _flags);
      break;
    }
  case ACCT_ATTR_PROMISE:
    {
      result = _acct.set_promise(txn, value);
      break;
    }
  }

  error_check(txn, result, 0, 0);

  commit_transaction(txn, 0, 0);

  writef("%03d success\n.\n", CODE_SUCCESS);
  return 0;
}

int bif::shell::cmd_acct_destroy(int argc, char **argv) {
  txn_t* txn;
  int result;
  bom::acct _acct;
  bom::acct_flag flags;

  if (argc != 2) {
    writef("%03d usage: acct_destroy acct\n.\n",
     CODE_ERR_USAGE);
    return 0;
  }
  
  _acct.set_id(argv[1]);
  
again:
  begin_transaction(txn, 0);

  result = acct.get_flags(txn, &flags);
  error_check(txn, result, 0, 0);
  
  if (!(flags & ACCT_FLAG_SUPER)) {
    abort_transaction(txn);
    writef("%03d no superacct permissions\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }
  
  if (!strcmp(acct.id, _acct.id)) {
    abort_transaction(txn);
    writef("%03d cannot delete self\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }
  
  {
    char exists;

    result = _acct.exists(txn, &exists);
    error_check(txn, result, 0, 0);
  
    if (!exists) {
      abort_transaction(txn);
      writef("%03d no such acct\n.\n", CODE_ERR_NOTFOUND);
      return 0;
    }
  }

  result = _acct.destroy(txn);
  error_check(txn, result, 0, 0);

  commit_transaction(txn, 0, 0);

  writef("%03d success\n.\n", CODE_SUCCESS);
  return 0;
}

int bif::shell::cmd_denom_destroy(int argc, char **argv) {
  txn_t* txn;
  int result;
  bom::denom denom;
  bom::acct_flag flags;

  if (argc != 2) {
    writef("%03d usage: denom_destroy denom\n.\n",
     CODE_ERR_USAGE);
    return 0;
  }
  
  denom.set_id(argv[1]);
  
again:
  begin_transaction(txn, 0);

  result = acct.get_flags(txn, &flags);
  error_check(txn, result, 0, 0);
  
  if (!(flags & ACCT_FLAG_SUPER)) {
    abort_transaction(txn);
    writef("%03d no superacct permissions\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }
  
  {
    char exists;

    result = denom.exists(txn, &exists);
    error_check(txn, result, 0, 0);
  
    if (!exists) {
      abort_transaction(txn);
      writef("%03d no such acct\n.\n", CODE_ERR_NOTFOUND);
      return 0;
    }
  }

  result = denom.destroy(txn);
  error_check(txn, result, 0, 0);

  commit_transaction(txn, 0, 0);

  writef("%03d success\n.\n", CODE_SUCCESS);
  return 0;
}


int bif::shell::cmd_login(int argc, char **argv) {
  txn_t *txn;
  int result;
  bom::acct _acct;

  if (argc != 2) {
    writef("usage: login acctid\n.\n", CODE_ERR_USAGE);
    return 0;
  }

  _acct.set_id(argv[1]);

again:
  begin_transaction(txn, 0);
  
  {
    char exists;

    result = _acct.exists(txn, &exists);
    error_check(txn, result, 0, 0);

    if (!exists) {
      abort_transaction(txn);
      writef("%03d acct does not exist\n.\n", CODE_ERR_NOTFOUND);
      return 0;
    }
  }

  {
    char success;

    result = _acct.create_challenge(txn, &challenge, &success);
    error_check(txn, result, 0, 0);

    if (!success) {
      abort_transaction(txn);
      writef("%03d database error: cannot make challenge\n.\n", CODE_ERR_DB);
      return 0;
    }
  }

  commit_transaction(txn, 0, 0);
  
  acct.set_id(_acct.id);
  status = 0;
  
  {
    char *schallenge;
    bz_to_string(challenge, &schallenge);
    writef("%03d challenge\n%s\n.\n", CODE_SUCCESS, schallenge);
    bom_os_free(schallenge);
  }
  return 0;
}

int bif::shell::cmd_respond(int argc, char **argv) {
  txn_t *txn;
  int result;
  dmath::bz response;

  if (argc != 2) {
    writef("%03d usage: respond signature\n.\n", CODE_ERR_USAGE);
    return 0;
  }
  if (status) {
    writef("%03d already logged on\n.\n", CODE_ERR_INVALID);
    return 0;
  }
  if (!acct.id) {
    writef("%03d challenge not issued\n.\n", CODE_ERR_INVALID);
    return 0;
  }

  result = string_to_bz(argv[1], &response);
  if (result) {
    writef("%03d invalid input (signature must be hex string)\n.\n",
     CODE_ERR_INVALID);
    return 0;
  }
  
again:
  begin_transaction(txn, 0);

  {
    char success;

    result = acct.verify_reponse(txn, challenge, response, &success);
    error_check(txn, result, 0, 0);
  
    if (!success) {
      abort_transaction(txn);
      acct.set_id(NULL);
      status = 0;
      writef("%03d cannot verify signature\n.\n", CODE_ERR_NOAUTH);
      return 0;
    }
  }
  
  commit_transaction(txn, 0, 0);

  status = STATUS_AUTHENTICATED;
  writef("%03d response accepted\n.\n", CODE_SUCCESS);
  return 0;
}

int bif::shell::cmd_acct_read(int argc, char **argv) {
  txn_t *txn;
  int result;
  bom::acct _acct;

  if (argc != 2) {
    writef("%03d usage: acct_read acctid\n.\n", CODE_ERR_USAGE);
    return 0;
  }

  _acct.set_id(argv[1]);

again:
  begin_transaction(txn, 0);

  {
    char exists;

    result = _acct.exists(txn, &exists);
    error_check(txn, result, 0, 0);

    if (!exists) {
      abort_transaction(txn);
      writef("%03d acct does not exist\n.\n", CODE_ERR_NOTFOUND);
      return 0;
    }
  }

  result = _acct.get(txn,
   ACCT_ATTR_FLAGS | ACCT_ATTR_PUBLIC | ACCT_ATTR_MODULUS | ACCT_ATTR_PROMISE);
  error_check(txn, result, 0, 0);

  {
    bom::acct_flag flags;

    result = acct.get_flags(txn, &flags);
    error_check(txn, result, 0, 0);

    if (flags & ACCT_FLAG_SUPER) {
      result = _acct.get(txn, ACCT_ATTR_SECRET);
      if (result != DB_NOTFOUND) {
        error_check(txn, result, 0, 0);
      } else {
	delete _acct._secret;
	_acct._secret = NULL;
      }
    }
  }

  commit_transaction(txn, 0, 0);
  
  {
    char *spublic, *smodulus, *sflags, *seflags;
    char *spromise, *sid;

    bz_to_string(*_acct._public, &spublic);
    bz_to_string(*_acct._modulus, &smodulus);
    acct_flags_to_string(_acct._flags, &sflags);
    encode_string(1, &sflags, &seflags);
    encode_string(1, &_acct._promise, &spromise);
    encode_string(1, &_acct.id, &sid);

    writef("%03d acct attributes follow\n"
    "id %s\nflags %s\npromise %s\npublic %s\nmodulus %s\n",
     CODE_SUCCESS, sid, seflags, spromise, spublic, smodulus);

    if (_acct._secret) {
      char *ssecret;
      bz_to_string(*_acct._secret, &ssecret);
      writef("secret %s\n", ssecret);
      bom_os_free(ssecret);
    }
    
    writef(".\n");

    bom_os_free(spublic);
    bom_os_free(smodulus);
    bom_os_free(sflags);
    bom_os_free(spromise);
    bom_os_free(sid);
  }
  return 0;
}

int bif::shell::cmd_balance(int argc, char **argv) {
  txn_t* txn;
  int result;
  bom::acct _acct;
  bom::value *vl = NULL;

  if (argc != 1 && argc != 2) {
    writef("%03d usage: balance [acct]\n.\n", CODE_ERR_USAGE);
    return 0;
  }
  
  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }

again:
  begin_transaction(txn, 0);
  
  if (argc == 2 && strcmp(acct.id, argv[1])) {
    result = acct.get(txn, ACCT_ATTR_FLAGS);
    error_check(txn, result, 0, 0);
    
    if (!(acct._flags & ACCT_FLAG_SUPER)) {
      abort_transaction(txn);
      writef("%03d permission denied\n.\n", CODE_ERR_NOAUTH);
      return 0;
    }
    
    _acct.set_id(argv[1]);

    char exists;
    result = _acct.exists(txn, &exists);
    error_check(txn, result, 0, 0);

    if (!exists) {
      abort_transaction(txn);
      writef("%03d acct not found\n.\n", CODE_ERR_NOTFOUND);
      return 0;
    }
  } else
    _acct.set_id(acct.id);

  result = _acct.get_portfolio(txn, &vl);
  error_check(txn, result, 0, 0);

  commit_transaction(txn, if (vl) delete vl, 0);
  
  writef("%03d balance follows\n", CODE_SUCCESS);

  for (bom::value *v = vl; v; v = v->next) {
    char *svalue;
    value_to_string(*v, &svalue);
    writef("%s\n", svalue);
    bom_os_free(svalue);
  }
 
  writef(".\n");

  if (vl)
    delete vl;
  return 0;
}


int bif::shell::cmd_note_read(int argc, char **argv) {
  txn_t* txn;
  int result;
  dmath::bz note_challenge, response;
  bom::value *vl, *v;
  bom::note note;
  bom::acct issuer;

  if (argc != 2 && argc != 3) {
    writef("%03d usage: note_read noteid [response]\n.\n",
     CODE_ERR_USAGE);
    return 0;
  }
  
  if (!status && argc == 2) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }
  
  note.set_id(argv[1]);

again:
  begin_transaction(txn, 0);

  {
    char exists;

    result = note.exists(txn, &exists);
    error_check(txn, result, 0, 0);

    if (!exists) {
      abort_transaction(txn);
      writef("%03d no such note\n.\n", CODE_ERR_NOTFOUND);
      return 0;
    }
  }

  result = note.get_challenge(txn, &note_challenge);
  error_check(txn, result, 0, 0);

  result = note.get_issuer(txn, &issuer);
  error_check(txn, result, 0, 0);

  if (argc == 2) {
    if (strcmp(issuer.id, acct.id)) {
      abort_transaction(txn);
      writef("%03d not your note\n.\n", CODE_ERR_NOAUTH);
      return 0;
    }
  } else {
    result = string_to_bz(argv[2], &response);
    if (result) {
      abort_transaction(txn);
      writef("%03d invalid input (signature must be hex string)\n.\n",
       CODE_ERR_INVALID);
      return 0;
    }

    {
      char success;

      result = note.verify_reponse(txn, note_challenge, response, &success);
      error_check(txn, result, 0, 0);

      if (!success) {
        abort_transaction(txn);
        writef("%03d bad note signature\n.\n", CODE_ERR_NOAUTH);
        return 0;
      }
    }
  }

  result = note.get_portfolio(txn, &vl);
  error_check(txn, result, 0, 0);

  commit_transaction(txn, 0, 0);
  
  writef("%03d note attributes follow\n", CODE_SUCCESS);
  
  writef("id %s\n", note.id);
  writef("issuer %s\n", issuer.id);
  {
    char *schallenge;
    bz_to_string(note_challenge, &schallenge);
    writef("challenge %s\n", schallenge);
    bom_os_free(schallenge);
  }

  for (v = vl; v; v = v->next) {
    char *svalue;
    value_to_string(*v, &svalue);
    writef("value %s\n", svalue);
    bom_os_free(svalue);
  }
 
  writef(".\n");

  delete vl;
  return 0;
}

int bif::shell::cmd_credit(int argc, char **argv) {
  txn_t* txn;
  int result;
  bom::acct _acct;
  bom::value *vl = NULL;

  if (argc < 3) {
    writef("%03d usage: credit acct value ...\n.\n", CODE_ERR_USAGE);
    return 0;
  }

  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }
  
  _acct.set_id(argv[1]);
  
  for (int i = 2; i < argc; i++) {
    bom::value *v = new bom::value;
    v->next = vl;
    vl = v;

    result = string_to_value(argv[i], v);
    if (result) {
      delete vl;
      writef("%03d invalid input\n.\n", CODE_ERR_INVALID);
      return 0;
    }

    if (v->quant <= 0) {
      delete vl;
      writef("%03d invalid input\n.\n", CODE_ERR_INVALID);
      return 0;
    }
    
    for (bom::value *w = v->next; w; w = w->next) {
      if (!strcmp(w->denom.id, v->denom.id)) {
        w->quant += v->quant;
        vl = v->next;
        v->next = NULL;
        delete v;
        break;
      }
    }
  }
  
again:
  begin_transaction(txn, 0);
  
  {
    char exists;

    result = _acct.exists(txn, &exists);
    error_check(txn, result, 0, delete vl);
    
    if (!exists) {
      abort_transaction(txn);
      delete vl;
      writef("%03d no such acct\n.\n", CODE_ERR_NOTFOUND);
      return 0;
    }
  }

  {
    bom::acct_flag flags;

    result = acct.get_flags(txn, &flags);
    error_check(txn, result, 0, delete vl);
  
    if (!(flags & ACCT_FLAG_SUPER)) {
      abort_transaction(txn);
      delete vl;
      writef("%03d permission denied\n.\n", CODE_ERR_NOAUTH);
      return 0;
    }
  }

  for (bom::value *v = vl; v; v = v->next) {
    char exists;

    result = v->denom.exists(txn, &exists);
    error_check(txn, result, 0, delete vl);

    if (!exists) {
      abort_transaction(txn);
      delete vl;
      writef("%03d denom not found\n.\n", CODE_ERR_NOTFOUND);
      return 0;
    }
  }

  for (bom::value *v = vl; v; v = v->next) {
    char success;
    
    result = _acct.credit(txn, *v, &success);
    error_check(txn, result, 0, delete vl);
  }

  commit_transaction(txn, 0, delete vl);

  writef("%03d values credited follow\n", CODE_SUCCESS);
  for (bom::value *v = vl; v; v = v->next) {
    char *svalue;
    value_to_string(*v, &svalue);
    writef("%s\n", svalue);
    bom_os_free(svalue);
  }
  writef(".\n");

  delete vl;
  return 0;
}

int bif::shell::cmd_debit(int argc, char **argv) {
  txn_t* txn;
  int result;
  bom::acct _acct;
  bom::value *vl = NULL;

  if (argc < 3) {
    writef("%03d usage: debit acct value ...\n.\n", CODE_ERR_USAGE);
    return 0;
  }

  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }
  
  _acct.set_id(argv[1]);
  
  for (int i = 2; i < argc; i++) {
    bom::value *v = new bom::value;
    v->next = vl;
    vl = v;

    result = string_to_value(argv[i], v);
    if (result) {
      delete vl;
      writef("%03d invalid input\n.\n", CODE_ERR_INVALID);
      return 0;
    }

    if (v->quant <= 0) {
      delete vl;
      writef("%03d invalid input\n.\n", CODE_ERR_INVALID);
      return 0;
    }
    
    for (bom::value *w = v->next; w; w = w->next) {
      if (!strcmp(w->denom.id, v->denom.id)) {
        w->quant += v->quant;
        vl = v->next;
        v->next = NULL;
        delete v;
        break;
      }
    }
  }
  
again:
  begin_transaction(txn, 0);
  
  {
    char exists;

    result = _acct.exists(txn, &exists);
    error_check(txn, result, 0, delete vl);
    
    if (!exists) {
      abort_transaction(txn);
      delete vl;
      writef("%03d no such acct\n.\n", CODE_ERR_NOTFOUND);
      return 0;
    }
  }

  {
    bom::acct_flag flags;

    result = acct.get_flags(txn, &flags);
    error_check(txn, result, 0, delete vl);
    
    if (!(flags & ACCT_FLAG_SUPER)) {
      abort_transaction(txn);
      delete vl;
      writef("%03d permission denied\n.\n", CODE_ERR_NOAUTH);
      return 0;
    }
  }

  for (bom::value *v = vl; v; v = v->next) {
    char exists;

    result = v->denom.exists(txn, &exists);
    error_check(txn, result, 0, delete vl);

    if (!exists) {
      abort_transaction(txn);
      delete vl;
      writef("%03d denom not found\n.\n", CODE_ERR_NOTFOUND);
      return 0;
    }
  }

  for (bom::value *v = vl; v; v = v->next) {
    result = _acct.debit_forced(txn, *v);
    error_check(txn, result, 0, delete vl);
  }

  commit_transaction(txn, 0, delete vl);

  writef("%03d values debited follow\n", CODE_SUCCESS);
  for (bom::value *v = vl; v; v = v->next) {
    char *svalue;
    value_to_string(*v, &svalue);
    writef("%s\n", svalue);
    bom_os_free(svalue);
  }
  writef(".\n");

  delete vl;
  return 0;
}

int bif::shell::cmd_note_withdraw(int argc, char **argv) {
  txn_t* txn;
  int result;
  bom::note note;
  dmath::bz note_challenge;
  bom::value *vl = NULL;

  if (argc < 2) {
    writef("%03d usage: note_withdraw value ...\n.\n", CODE_ERR_USAGE);
    return 0;
  }

  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }
  
  for (int i = 1; i < argc; i++) {
    bom::value *v = new bom::value;
    v->next = vl;
    vl = v;

    result = string_to_value(argv[i], v);
    if (result) {
      delete vl;
      writef("%03d invalid input\n.\n", CODE_ERR_INVALID);
      return 0;
    }

    if (v->quant <= 0) {
      delete vl;
      writef("%03d invalid input\n.\n", CODE_ERR_INVALID);
      return 0;
    }
    
    for (bom::value *w = v->next; w; w = w->next) {
      if (!strcmp(w->denom.id, v->denom.id)) {
        w->quant += v->quant;
        vl = v->next;
        v->next = NULL;
        delete v;
        break;
      }
    }
  }
  
  note.gen_id();

again:
  begin_transaction(txn, 0);

  if (parent) {
    for (bom::value *v = vl; v; v = v->next) {
      bom::value w;

      w.denom.set_id(v->denom.id);
      result = acct.get_value(txn, &w);
      error_check(txn, result, 0, delete vl);
    
      if (w.quant < v->quant) {
        abort_transaction(txn);
        delete vl;
        writef("%03d insufficient funds\n.\n", CODE_ERR_INSUFFICIENT);
        return 0;
      }
    }
  }

  result = note.set_issuer(txn, acct);
  error_check(txn, result, 0, delete vl);

  {
    char success;

    result = note.create_challenge(txn, &note_challenge, &success);
    error_check(txn, result, 0, delete vl);

    if (!success) {
      abort_transaction(txn);
      delete vl;
      writef("%03d database error: cannot make challenge\n.\n",
       CODE_ERR_DB);
      return 0;
    }
  }

  result = note.set_challenge(txn, note_challenge);
  error_check(txn, result, 0, delete vl);

  for (bom::value *v = vl; v; v = v->next) {
    char success;

    result = acct.debit(txn, *v, &success);
    error_check(txn, result, 0, delete vl);

    if (!success) {
      abort_transaction(txn);
      delete vl;
      writef("%03d insufficient funds\n.\n", CODE_ERR_INSUFFICIENT);
      return 0;
    }

    result = note.set_value(txn, *v);
    error_check(txn, result, 0, delete vl);
  }

  note._issuer = &acct;
  new_event(txn, result, note_create, note);
  note._issuer = NULL;
  error_check(txn, result, 0, delete vl);

  commit_transaction(txn, 0, delete vl);

  delete vl;
  
  writef("%03d noteid follows\n%s\n.\n", CODE_SUCCESS, note.id);
  return 0;
}

int bif::shell::cmd_note_list(int argc, char **argv) {
  txn_t* txn;
  int result;
  bom::acct _acct;
  bom::note *nl, *n;

  if (argc != 1 && argc != 2) {
    writef("%03d usage: note_list [acct]\n.\n", CODE_ERR_USAGE);
    return 0;
  }
  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }
  
again:
  begin_transaction(txn, 0);

  if (argc == 2 && strcmp(acct.id, argv[1])) {
    result = acct.get(txn, ACCT_ATTR_FLAGS);
    error_check(txn, result, 0, 0);
    
    if (!(acct._flags & ACCT_FLAG_SUPER)) {
      abort_transaction(txn);
      writef("%03d permission denied\n.\n", CODE_ERR_NOAUTH);
      return 0;
    }
    
    _acct.set_id(argv[1]);

    char exists;
    result = _acct.exists(txn, &exists);
    error_check(txn, result, 0, 0);

    if (!exists) {
      abort_transaction(txn);
      writef("%03d acct not found\n.\n", CODE_ERR_NOTFOUND);
      return 0;
    }
  } else
    _acct.set_id(acct.id);

  result = _acct.get_note_list(txn, &nl);
  error_check(txn, result, 0, 0);

  commit_transaction(txn, 0, 0);
  
  writef("%03d note list follows\n", CODE_SUCCESS);
  for (n = nl; n; n = n->next)
    writef("%s\n", n->id);
  writef(".\n");

  if (nl)
    delete nl;

  return 0;
}

int bif::shell::cmd_note_deposit(int argc, char **argv) {
  txn_t* txn;
  int result;
  bom::note note;
  dmath::bz note_challenge, response;
  bom::value *vl = NULL;

  if (argc != 3) {
    writef("%03d usage: note_deposit noteid signature\n.\n",
     CODE_ERR_USAGE);
    return 0;
  }
  
  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }

  note.set_id(argv[1]);

  result = string_to_bz(argv[2], &response);
  if (result) {
    writef("%03d invalid input\n.\n", CODE_ERR_INVALID);
    return 0;
  }

again:
  begin_transaction(txn, 0);
  
  {
    char exists;

    result = note.exists(txn, &exists);
    error_check(txn, result, 0, 0);

    if (!exists) {
      abort_transaction(txn);
      writef("%03d no such note\n.\n", CODE_ERR_NOTFOUND);
      return 0;
    }
  }
  
  result = note.get(txn, NOTE_ATTR_ISSUER);
  error_check(txn, result, 0, 0);

  result = note.get_challenge(txn, &note_challenge);
  error_check(txn, result, 0, 0);

  {
    char success;

    result = note.verify_reponse(txn, note_challenge, response, &success);
    error_check(txn, result, 0, 0);

    if (!success) {
      abort_transaction(txn);
      writef("%03d bad note signature\n.\n", CODE_ERR_NOAUTH);
      return 0;
    }
  }
    
  result = note.get_portfolio(txn, &vl);
  error_check(txn, result, 0, 0);

  for (bom::value *v = vl; v; v = v->next) {
    char success;

    result = acct.credit(txn, *v, &success);
    error_check(txn, result, delete vl, 0);
    
    if (!success) {
      abort_transaction(txn);
      delete vl;
      writef("%03d database error: invalid note\n.\n", CODE_ERR_DB);
      return 0;
    }
  }

  result = note.destroy(txn);
  error_check(txn, result, if (vl) delete vl, 0);

  commit_transaction(txn, if (vl) delete vl, 0);

  writef("%03d deposited; note values follow\n",
   CODE_SUCCESS);
  for (bom::value *v = vl; v; v = v->next) {
    char *svalue;
    value_to_string(*v, &svalue);
    writef("%s\n", svalue);
    bom_os_free(svalue);
  }
  writef(".\n");
  
  if (vl)
    delete vl;
  return 0;
}

int bif::shell::cmd_exit(int argc, char **argv) {
  int retval;
  
  writef("%03d goodbye\n.\n", CODE_GOODBYE);

  delete this;
  thread_exit();
  return 0;
}

int bif::shell::cmd_shutdown(int argc, char **argv) {
  int result;
  bom::acct_flag flags;
  txn_t *txn;

  if (argc != 1) {
    writef("%03d usage: shutdown\n.\n", CODE_ERR_USAGE);
    return 0;
  }

  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }

again:
  begin_transaction(txn, 0);
  
  result = acct.get_flags(txn, &flags);
  error_check(txn, result, 0, 0);
  
  if (!(flags & ACCT_FLAG_SUPER)) {
    abort_transaction(txn);
    writef("%03d must be superacct to shutdown\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }
    
  commit_transaction(txn, 0, 0);

  for (bif::shell *c = sv->client_list; c; c = c->next) {
    if (c != this)
      c->lock();
  }

  for (bif::shell *c = sv->client_list; c; c = c->next) {
    c->writef("%03d shutting down; goodbye\n.\n", CODE_GOODBYE);
    if (c != this)
      c->cancel();
    c->close();
  }
  
  for (bif::shell *c = sv->client_list; c; c = c->next) {
    if (c != this)
      c->unlock();
  }

  bif_shutdown();
  thread_exit();
  return 0;
}

int bif::shell::cmd_event_send(int argc, char **argv) {
  txn_t* txn;
  int result;
  bom::acct from, to;
  char *data;
  bom::event event_sent, event_received;

  if (argc != 3) {
    writef("%03d usage: event_send recipient data\n.\n",
     CODE_ERR_USAGE);
    return 0;
  }
  
  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }

  from.set_id(acct.id);
  to.set_id(argv[1]);
  data = argv[2];

again:
  begin_transaction(txn, 0);
  
  {
    char exists;
    result = to.exists(txn, &exists);
    error_check(txn, result, 0, 0);
    
    if (!exists) {
      writef("%03d recipient not found\n.\n", CODE_ERR_NOTFOUND);
      return 0;
    }
  }
  
  result = event_new_event_sent(txn, &event_sent, from, to, data);
  error_check(txn, result, 0, 0);

  result = event_new_event_received(txn, &event_received, from, to, data);
  error_check(txn, result, 0, 0);

  commit_transaction(txn, 0, 0);
  
  writef("%03d event sent, event id follows\n%d\n.\n",
   CODE_SUCCESS, event_sent.id);
  return 0;
}

int bif::shell::cmd_event_list(int argc, char **argv) {
  txn_t* txn;
  int result;
  bom::event *el, *e;
  bom::event_id lb, ub;
  char has_lb = 0, has_ub = 0;

  if (argc != 1 && argc != 2 && argc != 3) {
    writef("%03d usage: event_list [lower-bound [upper-bound]]\n.\n",
     CODE_ERR_USAGE);
    return 0;
  }

  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }
  
  if (argc > 1) {
    has_lb = 1;
    if ((lb = (bom::event_id)strtoul(argv[1], NULL, 10)) < 1) {
      writef("%03d invalid input (lower-bound)\n.\n", CODE_ERR_INVALID);
      return 0;
    }
  }
  if (argc > 2) {
    has_ub = 1;
    if ((ub = (bom::event_id)strtoul(argv[2], NULL, 10)) < 1) {
      writef("%03d invalid input (upper-bound)\n.\n", CODE_ERR_INVALID);
      return 0;
    }
  }
  
again:
  begin_transaction(txn, 0);

  result = acct.get_event_list(txn, &el);
  error_check(txn, result, 0, 0);

  commit_transaction(txn, 0, 0);
  
  writef("%03d event list follows\n", CODE_SUCCESS);

  for (e = el; e; e = e->next) {
    if (has_lb && e->id < lb)
      continue;
    if (has_ub && e->id > ub)
      continue;
    writef("%d\n", e->id);
  }

  writef(".\n");

  if (el)
    delete el;
  return 0;
}

int bif::shell::cmd_event_last(int argc, char **argv) {
  txn_t* txn;
  int result;
  bom::event event;

  if (argc != 1) {
    writef("%03d usage: event_last\n.\n",
     CODE_ERR_USAGE);
    return 0;
  }

  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }
  
again:
  begin_transaction(txn, 0);

  {
    bom::acct_flag flags;

    result = acct.get_flags(txn, &flags);
    error_check(txn, result, 0, 0);

    if (!(flags & ACCT_FLAG_SUPER)) {
      writef("%03d permission denied\n.\n", CODE_ERR_NOAUTH);
      return 0;
    }
  }

  result = get_last_event(txn, &event);
  if (result == DB_NOTFOUND) {
    writef("%03d no events in database\n.\n", CODE_ERR_NOTFOUND);
    return 0;
  }
  error_check(txn, result, 0, 0);

  commit_transaction(txn, 0, 0);
  
  writef("%03d last event follows\n%d\n.\n", CODE_SUCCESS, event.id);
  return 0;
}

int bif::shell::cmd_event_read(int argc, char **argv) {
  txn_t *txn;
  int result;
  bom::acct_flag flags;
  bom::event event;
  bom::event_type type;
  bom::acct *ul;
  char *data;

  if (argc != 2) {
    writef("%03d usage: event_read event\n.\n",
     CODE_ERR_USAGE);
    return 0;
  }
  
  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }
  
  event.set_id(strtoul(argv[1], NULL, 10));
  if (event.id < 1) {
    writef("%03d invalid input\n.\n", CODE_ERR_INVALID);
    return 0;
  }

again:
  begin_transaction(txn, 0);

  {
    char exists;

    result = event.exists(txn, &exists);
    error_check(txn, result, 0, 0);

    if (!exists) {
      abort_transaction(txn);
      writef("%03d no such event\n.\n", CODE_ERR_NOTFOUND);
      return 0;
    }
  }

  ul = NULL;
  result = event.get_acct_list(txn, &ul);
  error_check(txn, result, 0, 0);

  result = acct.get_flags(txn, &flags);
  error_check(txn, result, if (ul) delete ul, 0);
  
  {
    char permission = FALSE;
    for (bom::acct *u = ul; u && !permission; u = u->next)
      if (!strcmp(acct.id, u->id) || (flags & ACCT_FLAG_SUPER))
	permission = TRUE;
      
    if (!permission) {
      abort_transaction(txn);
      if (ul)
        delete ul;
      writef("%03d permission denied\n.\n", CODE_ERR_NOAUTH);
      return 0;
    }
  }

  result = event.get_type(txn, &type);
  error_check(txn, result, if (ul) delete ul, 0);

  result = event.get_data(txn, &data);
  error_check(txn, result, if (ul) delete ul, 0);
  
  commit_transaction(txn, if (ul) delete ul; bom_os_free(data), 0);
  
  writef("%03d event attributes follow\n", CODE_SUCCESS);
  writef("id %d\n", event.id);
  writef("type %s\n", event_name[type]);
  writef("%s\n.\n", data);
  
  bom_os_free(data);

  if (ul)
    delete ul;
  return 0;
}

int bif::shell::cmd_denom_read(int argc, char **argv) {
  txn_t *txn;
  int result;
  bom::denom denom;
  bom::acct issuer;
  char *promise;
  char *spromise;

  if (argc != 2) {
    writef("%03d usage: denom_read denomid\n.\n",
     CODE_ERR_USAGE);
    return 0;
  }
  
  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }
  
  denom.set_id(argv[1]);

again:
  begin_transaction(txn, 0);

  {
    char exists;

    result = denom.exists(txn, &exists);
    error_check(txn, result, 0, 0);

    if (!exists) {
      abort_transaction(txn);
      writef("%03d no such denom\n.\n", CODE_ERR_NOTFOUND);
      return 0;
    }
  }

  result = denom.get_issuer(txn, &issuer);
  error_check(txn, result, 0, 0);

  result = denom.get_promise(txn, &promise);
  error_check(txn, result, 0, 0);

  encode_string(1, &promise, &spromise);
  bom_os_free(promise);
  
  commit_transaction(txn, 0, bom_os_free(spromise));

  {
    char *sid;

    encode_string(1, &denom.id, &sid);
    writef("%03d denom attributes follow\n", CODE_SUCCESS);
    writef("id %s\n", sid);
    writef("issuer %s\n", issuer.id);
    writef("promise %s\n", spromise);
    writef(".\n");

    bom_os_free(sid);
  }

  bom_os_free(spromise);
  return 0;
}

int bif::shell::cmd_denom_list_all(int argc, char **argv) {
  txn_t* txn;
  int result;
  bom::denom *dl = NULL, *d;

  if (argc != 1) {
    writef("%03d usage: denom_list_all\n.\n", CODE_ERR_USAGE);
    return 0;
  }
  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }
  
again:
  begin_transaction(txn, 0);

  result = get_denom_list(txn, &dl);
  error_check(txn, result, 0, 0);

  commit_transaction(txn, if (dl) delete dl, 0);
  
  writef("%03d denom list follows\n", CODE_SUCCESS);
  for (d = dl; d; d = d->next)
    writef("%s\n", d->id);
  writef(".\n");

  if (dl)
    delete dl;
  return 0;
}

int bif::shell::cmd_denom_search(int argc, char **argv) {
  txn_t* txn;
  int result;
  bom::denom *dl = NULL, *d;

  if (argc != 2) {
    writef("%03d usage: denom_search pattern\n.\n", CODE_ERR_USAGE);
    return 0;
  }
  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }
  
again:
  begin_transaction(txn, 0);

  result = search_denom_list(txn, argv[1], &dl);
  error_check(txn, result, 0, 0);
  
  commit_transaction(txn, if (dl) delete dl, 0);
  
  writef("%03d denom list follows\n", CODE_SUCCESS);
  for (d = dl; d; d = d->next)
    writef("%s\n", d->id);
  writef(".\n");

  if (dl)
    delete dl;
  return 0;
}

int bif::shell::cmd_denom_list(int argc, char **argv) {
  txn_t* txn;
  int result;
  bom::acct _acct;
  bom::denom *dl = NULL, *d;

  if (argc != 1 && argc != 2) {
    writef("%03d usage: denom_list [acct]\n.\n", CODE_ERR_USAGE);
    return 0;
  }
  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }
  
again:
  begin_transaction(txn, 0);

  if (argc == 2 && strcmp(acct.id, argv[1])) {
    result = acct.get(txn, ACCT_ATTR_FLAGS);
    error_check(txn, result, 0, 0);
    
    if (!(acct._flags & ACCT_FLAG_SUPER)) {
      abort_transaction(txn);
      writef("%03d permission denied\n.\n", CODE_ERR_NOAUTH);
      return 0;
    }
    
    _acct.set_id(argv[1]);

    char exists;
    result = _acct.exists(txn, &exists);
    error_check(txn, result, 0, 0);

    if (!exists) {
      abort_transaction(txn);
      writef("%03d acct not found\n.\n", CODE_ERR_NOTFOUND);
      return 0;
    }
  } else
    _acct.set_id(acct.id);

  result = _acct.get_denom_list(txn, &dl);
  error_check(txn, result, 0, 0);

  commit_transaction(txn, if (dl) delete dl, 0);
  
  writef("%03d denom list follows\n", CODE_SUCCESS);
  for (d = dl; d; d = d->next)
    writef("%s\n", d->id);
  writef(".\n");

  if (dl)
    delete dl;
  return 0;
}

int bif::shell::cmd_connect(int argc, char **argv) {
  bif::site site;

  if (argc != 2)  {
    writef("%03d usage: connect site\n.\n",
     CODE_ERR_USAGE);
    return 0;
  }
  
  if (int result = string_to_site(argv[1], &site)) {
    writef("%03d site must be of the form host:port\n.\n",
     CODE_ERR_INVALID);
    return 0;
  }
  
  bif::session *_session;
  _session = new bif::session(this);

  if (int result = _session->client.connect(site)) {
    writef("%03d cannot connect to site\n.\n", CODE_ERR_NET);
    delete _session;
    return 0;
  }
  
  session = _session;
  writef("%03d success; session id follows\n%s\n.\n",
   CODE_SUCCESS, session->id);
  return 0;
}
    
int bif::shell::cmd_reconnect(int argc, char **argv) {
  bif::session *_session;

  if (argc != 2)  {
    writef("%03d usage: reconnect sessionid\n.\n",
     CODE_ERR_USAGE);
    return 0;
  }
  
  _session = find_session(argv[1]);
  if (!_session) {
    writef("%03d no such session\n.\n", CODE_ERR_NOTFOUND);
    return 0;
  }
  
  session = _session;

  writef("%03d success; reconnected to session\n.\n", CODE_SUCCESS);
  return 0;
}
    
int bif::shell::cmd_disconnect(int argc, char **argv) {
  bif::session *_session;

  if (argc != 1)  {
    writef("%03d usage: disconnect\n.\n",
     CODE_ERR_USAGE);
    return 0;
  }
  
  if (!session) {
    writef("%03d not in a session\n.\n", CODE_ERR_NOTFOUND);
    return 0;
  }
  
  _session = session;
  session = NULL;
  delete _session;

  writef("%03d success; disconnected\n.\n", CODE_SUCCESS);
  return 0;
}
    
int bif::shell::cmd_help(int argc, char **argv) {
  if (argc != 1)  {
    writef("%03d usage: help\n.\n",
     CODE_ERR_USAGE);
    return 0;
  }
  
  writef("%03d success; command list follows\n", CODE_SUCCESS);

  for (int i = 0; *command_table[i].command; i++)
    writef("%s\n", command_table[i].command);
  writef(".\n");
}


int bif::shell::cmd_txn_create(int argc, char **argv) {
  txn_t *txn;
  int result;
  bom::acct_flag flags;
  bom::tid tid;

  if (argc != 1) {
    writef("%03d usage: txn_create\n.\n", CODE_ERR_USAGE);
    return 0;
  }
  
  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }

  if (parent) {
    writef("%03d already in a transaction\n.\n", CODE_ERR_INVALID);
    return 0;
  }

  tid.gen_id();

again:
  begin_transaction(txn, 0);

  result = acct.get_flags(txn, &flags);
  error_check(txn, result, 0, 0);

  if (!(flags & ACCT_FLAG_TXN) && !(flags & ACCT_FLAG_SUPER)) {
    writef("%03d permission denied\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }
  
  result = tid.set_acct(txn, acct);
  error_check(txn, result, 0, 0);

  result = tid.set_command(txn, "");
  error_check(txn, result, 0, 0);
  
  parent = txn;
  writef("%03d success; txn id follows\n%s\n.\n", CODE_SUCCESS, tid.id);
}

int bif::shell::cmd_txn_commit(int argc, char **argv) {
  if (argc != 1) {
    writef("%03d usage: txn_commit\n.\n", CODE_ERR_USAGE);
    return 0;
  }
  
  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }

  if (!parent) {
    writef("%03d not in a transaction\n.\n", CODE_ERR_INVALID);
    return 0;
  }
  
  if (int result = txn_commit(parent)) {
    txn_abort(parent);
    parent = NULL;
    writef("%03d transaction aborted\n.\n", CODE_ERR_ABORTED);
    return 0;
  }
  
  parent = NULL;
  writef("%03d success\n.\n", CODE_SUCCESS);
}

int bif::shell::cmd_txn_abort(int argc, char **argv) {
  if (argc != 1) {
    writef("%03d usage: txn_abort\n.\n", CODE_ERR_USAGE);
    return 0;
  }
  
  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }

  if (!parent) {
    writef("%03d not in a transaction\n.\n", CODE_ERR_INVALID);
    return 0;
  }
  
  txn_abort(parent);
  parent = NULL;
  
  writef("%03d success\n.\n", CODE_SUCCESS);
}

int bif::shell::cmd_txn_read(int argc, char **argv) {
  txn_t *txn;
  int result;
  bom::acct_flag flags;
  bom::tid tid;
  char exists;

  if (argc != 2) {
    writef("%03d usage: txn_read txn\n.\n",
     CODE_ERR_USAGE);
    return 0;
  }
  
  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }
  
  tid.set_id(argv[1]);

again:
  begin_transaction(txn, 0);

  result = tid.exists(txn, &exists);
  error_check(txn, result, 0, 0);

  if (!exists) {
    abort_transaction(txn);
    writef("%03d no such txn\n.\n", CODE_ERR_NOTFOUND);
    return 0;
  }

  result = tid.get(txn, TID_ATTR_ACCT);
  error_check(txn, result, 0, 0);

  if (strcmp(tid.acct->id, acct.id)) {
    result = acct.get_flags(txn, &flags);
    error_check(txn, result, 0, 0);

    if (!(flags & ACCT_FLAG_SUPER)) {
      abort_transaction(txn);
      writef("%03d permission denied\n.\n", CODE_ERR_NOAUTH);
      return 0;
    }
  }

  commit_transaction(txn, 0, 0);
  
  writef("%03d txn attributes follow\n", CODE_SUCCESS);
  writef("id %s\n", tid.id);
  writef("acct %s\n", tid.acct->id);
  writef(".\n");
  
  return 0;
}

int bif::shell::cmd_offer_list(int argc, char **argv) {
  txn_t* txn;
  int result;
  bom::acct _acct;
  bom::offer *ol, *o;

  if (argc != 1 && argc != 2) {
    writef("%03d usage: offer_list [acct]\n.\n", CODE_ERR_USAGE);
    return 0;
  }
  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }
  
again:
  begin_transaction(txn, 0);

  if (argc == 2 && strcmp(acct.id, argv[1])) {
    result = acct.get(txn, ACCT_ATTR_FLAGS);
    error_check(txn, result, 0, 0);
    
    if (!(acct._flags & ACCT_FLAG_SUPER)) {
      abort_transaction(txn);
      writef("%03d permission denied\n.\n", CODE_ERR_NOAUTH);
      return 0;
    }
    
    _acct.set_id(argv[1]);

    char exists;
    result = _acct.exists(txn, &exists);
    error_check(txn, result, 0, 0);

    if (!exists) {
      abort_transaction(txn);
      writef("%03d acct not found\n.\n", CODE_ERR_NOTFOUND);
      return 0;
    }
  } else
    _acct.set_id(acct.id);

  result = acct.get_offer_list(txn, &ol);
  error_check(txn, result, 0, 0);

  commit_transaction(txn, if (ol) delete ol, 0);
  
  writef("%03d offer list follows\n", CODE_SUCCESS);
  for (o = ol; o; o = o->next)
    writef("%s\n", o->id);
  writef(".\n");

  if (ol)
    delete ol;
  return 0;
}


int bif::shell::cmd_offer_list_by_denom(int argc, char **argv) {
  txn_t* txn;
  int result;
  bom::offer *ol = NULL;
  bom::denom ask, bid;

  if (argc != 3) {
    writef("%03d usage: offer_list_by_denom ask_denom bid_denom\n.\n",
     CODE_ERR_USAGE);
    return 0;
  }
  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }

  ask.set_id(argv[1]);
  bid.set_id(argv[2]);
  
again:
  begin_transaction(txn, 0);

  {
    char exists;
    result = ask.exists(txn, &exists);
    error_check(txn, result, 0, 0);

    if (!exists) {
      abort_transaction(txn);
      writef("%03d ask_denom does not exist\n.\n", CODE_ERR_NOTFOUND);
      return 0;
    }
  }
    
  {
    char exists;
    result = bid.exists(txn, &exists);
    error_check(txn, result, 0, 0);

    if (!exists) {
      abort_transaction(txn);
      writef("%03d bid_denom does not exist\n.\n", CODE_ERR_NOTFOUND);
      return 0;
    }
  }
    
  result = find_all_offers(txn, ask, bid, &ol);
  error_check(txn, result, 0, 0);

  commit_transaction(txn, if (ol) delete ol, 0);

  bom::offer *rol = NULL, *no;
  for (bom::offer *o = ol; o; o = no) {
    no = o->next;
    o->next = rol;
    rol = o;
  }
  ol = rol;
  
  writef("%03d offer list follows\n", CODE_SUCCESS);
  for (bom::offer *o = ol; o; o = o->next)
    writef("%s\n", o->id);
  writef(".\n");

  if (ol)
    delete ol;
  return 0;
}

int bif::shell::cmd_offer_destroy(int argc, char **argv) {
  txn_t* txn;
  int result;
  bom::offer offer;
  bom::value bid_value;

  if (argc != 2) {
    writef("%03d usage: offer_destroy offerid\n.\n", CODE_ERR_USAGE);
    return 0;
  }
  
  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }

  offer.set_id(argv[1]);

again:
  begin_transaction(txn, 0);
    
  {
    char exists;

    result = offer.exists(txn, &exists);
    error_check(txn, result, 0, 0);

    if (!exists) {
      abort_transaction(txn);
      writef("%03d no such offer\n.\n", CODE_ERR_NOTFOUND);
      return 0;
    }
  }

  result = offer.get(txn, OFFER_ATTR_ISSUER | OFFER_ATTR_BID_VALUE);
  error_check(txn, result, 0, 0);

  if (strcmp(offer.issuer->id, acct.id)) {
    result = acct.get(txn, ACCT_ATTR_FLAGS);
    error_check(txn, result, 0, 0);
      
    if (!(acct._flags & ACCT_FLAG_SUPER)) {
      abort_transaction(txn);
      writef("%03d not your offer\n.\n", CODE_ERR_NOAUTH);
      return 0;
    }
  }

  {
    char success;

    result = acct.credit(txn, *offer.bid_value, &success);
    error_check(txn, result, 0, 0);

    if (!success) {
      abort_transaction(txn);
      writef("%03d database error: cannot credit bid\n.\n", CODE_ERR_DB);
      return 0;
    }
  }

  result = offer.destroy(txn);
  error_check(txn, result, 0, 0);

  commit_transaction(txn, 0, 0);

  writef("%03d offer destroyed\n%s\n.\n", CODE_SUCCESS, offer.id);
  return 0;
}

int bif::shell::cmd_offer_create(int argc, char **argv) {
  txn_t* txn;
  int result;
  bom::offer offer;
  bom::value ask, bid;

  if (argc != 3) {
    writef("%03d usage: offer_create ask_value bid_value\n.\n",
     CODE_ERR_USAGE);
    return 0;
  }

  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }

  if (string_to_value(argv[1], &ask)) {
    writef("%03d invalid input: ask_value\n.\n", CODE_ERR_INVALID);
    return 0;
  }
  if (ask.quant <= 0) {
    writef("%03d invalid input: ask_value (must be > 0)\n.\n",
     CODE_ERR_INVALID);
    return 0;
  }
  if (string_to_value(argv[2], &bid)) {
    writef("%03d invalid input: bid_value\n.\n", CODE_ERR_INVALID);
    return 0;
  }
  if (bid.quant <= 0) {
    writef("%03d invalid input: bid_value (must be > 0)\n.\n",
     CODE_ERR_INVALID);
    return 0;
  }
  if (!strcmp(ask.denom.id, bid.denom.id)) {
    writef("%03d invalid input: offer must have different denoms\n.\n",
     CODE_ERR_INVALID);
    return 0;
  }
  
  offer.gen_id();

again:
  begin_transaction(txn, 0);

  {
    char success;

    result = acct.debit(txn, bid, &success);
    error_check(txn, result, 0, 0);

    if (!success) {
      abort_transaction(txn);
      writef("%03d database error: cannot debit bid\n.\n", CODE_ERR_DB);
      return 0;
    }
  }

  result = offer.set_issuer(txn, acct);
  error_check(txn, result, 0, 0);

  result = offer.set_bid_value(txn, bid);
  error_check(txn, result, 0, 0);

  result = offer.set_ask_value(txn, ask);
  error_check(txn, result, 0, 0);

  result = offer.set_index(txn, ask, bid);
  error_check(txn, result, 0, 0);

  offer.issuer = &acct;
  new_event(txn, result, offer_create, offer);
  offer.issuer = NULL;

  error_check(txn, result, 0, 0);

  commit_transaction(txn, 0, 0);

  writef("%03d offerid follows\n%s\n.\n", CODE_SUCCESS, offer.id);

  sv->market->signal(ask.denom, bid.denom);
  return 0;
}

int bif::shell::cmd_offer_read(int argc, char **argv) {
  txn_t* txn;
  int result;
  bom::offer offer;
  bom::acct issuer;
  bom::value ask, bid;

  if (argc != 2) {
    writef("%03d usage: offer_read offerid\n.\n", CODE_ERR_USAGE);
    return 0;
  }

  if (!status) {
    writef("%03d not authenticated\n.\n", CODE_ERR_NOAUTH);
    return 0;
  }

  offer.set_id(argv[1]);

again:
  begin_transaction(txn, 0);
  
  {
    char exists;

    result = offer.exists(txn, &exists);
    error_check(txn, result, 0, 0);
    
    if (!exists) {
      abort_transaction(txn);
      writef("%03d no such offer\n.\n", CODE_ERR_NOTFOUND);
      return 0;
    }
  }

  result = offer.get_issuer(txn, &issuer);
  error_check(txn, result, 0, 0);
  
  result = offer.get_bid_value(txn, &bid);
  error_check(txn, result, 0, 0);

  result = offer.get_ask_value(txn, &ask);
  error_check(txn, result, 0, 0);

  commit_transaction(txn, 0, 0);
  
  writef("%03d offer attributes follow\n", CODE_SUCCESS);

  writef("id %s\n", offer.id);

  if (!strcmp(issuer.id, acct.id)) {
    writef("issuer %s\n", issuer.id);
  }

  {
    char *sask;
    value_to_string(ask, &sask);
    writef("ask %s\n", sask);
    bom_os_free(sask);
  }
  {
    char *sbid;
    value_to_string(bid, &sbid);
    writef("bid %s\n", sbid);
    bom_os_free(sbid);
  }
  writef(".\n");
  return 0;
}

