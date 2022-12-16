// {HEADER}

#define _CLIENT_CC
#include "bom.h"
#include "bif.h"

bif::client::client() {
  site = NULL;
  messages = NULL;
  messages_tail = NULL;
  next = NULL;
}

bif::client::~client() {
  if (site)
    delete site;
  if (messages)
    delete messages;
  if (next)
    delete next;
}

int bif::client::connect(const bif::site &_site) {
  bif::message m;
  int result;
  struct sockaddr_in to_addr0;

  if ((s = ::socket(AF_INET, SOCK_STREAM, SOL_TCP)) < 0)
    return s;

  if ((result = find_host(_site.host, _site.port, &to_addr0)) < 0)
    return -1;

  if ((result = ::connect(s, (sockaddr *)&to_addr0, sizeof(sockaddr_in))) < 0)
    return -2;
  
  read_message(&m);
  if (m.code != CODE_WELCOME) {
    close();
    return -3;
  }
  
  site = new bif::site(_site);

  return 0;
}

int bif::client::close() {
  bif::socket::close();
  cancel();
}

#define BUFFER_LENGTH_BEGIN		64

int bif::client::read_one_message(bif::message *message) {
  char c, lc = '\0';
  int n;
  int length, alength;
  
  length = 0;

  if (message->desc)
    bom_os_free(message->desc);
  message->desc = (char *)bom_os_malloc(alength = BUFFER_LENGTH_BEGIN);

  while ((n = read(&c, 1)) == 1) {
    if (length >= alength)
      message->desc = (char *)bom_os_realloc(message->desc, alength *= 2);
    if (c == '\n')
      break;
    *(message->desc + length++) = c;
  }
  *(message->desc + length) = '\0';

  if (n != 1) {
    bom_os_free(message->desc);
    message->desc = NULL;
    return -1;
  }
  
  message->code = atoi(message->desc);

  length = 0;
  if (message->data)
    bom_os_free(message->data);
  message->data = (char *)bom_os_malloc(alength = BUFFER_LENGTH_BEGIN);

  while ((n = read(&c, 1)) == 1) {
    if (length >= alength)
      message->data = (char *)bom_os_realloc(message->data, alength *= 2);
    if (c == '\n' && ((length == 1 && message->data[0] == '.') ||
     (length > 1 && message->data[length - 1] == '.' &&
     message->data[length - 2] == '\n')))
      break;
    *(message->data + length++) = c;
  }

  if (n != 1) {
    bom_os_free(message->desc);
    message->desc = NULL;
    bom_os_free(message->data);
    message->data = NULL;
    return -1;
  }
  
  length--;
  *(message->data + length) = '\0';

  return 0;
}

int bif::client::read_message(bif::message *message) {
  bif::message *m;
  
  do {
    m = new bif::message;

    if (int result = read_one_message(m)) {
      delete m;
      return result;
    }

    if (m->type() == MESSAGE_ASYNC) {
      m->next = messages;
      messages = m;
    }
  } while (m->type() == MESSAGE_ASYNC);

  message->code = m->code;
  message->desc = bom_os_strdup(m->desc);
  message->data = bom_os_strdup(m->data);
  delete m;
  
  return 0;
}

int bif::client::cmd_login(const bom::acct &acct, dmath::bz *challenge) {
  bif::message m;

  writef("login %s\r\n", acct.id);
  if (int result = read_message(&m))
    return result;
  if (m.code != CODE_SUCCESS)
    return m.code;
  
  string_to_bz(m.data, challenge);
  return 0;
}

int bif::client::cmd_respond(const dmath::bz &response) {
  bif::message m;

  {
    char *sresponse;
    bz_to_string(response, &sresponse);
    writef("respond %s\r\n", sresponse);
    bom_os_free(sresponse);
  }

  if (int result = read_message(&m))
    return result;
  if (m.code != CODE_SUCCESS)
    return m.code;
  
  return 0;
}

int bif::client::cmd_event_send(const bom::acct &to, char *data, bom::event *event) {
  bif::message m;

  {
    char *edata;
    encode_string(1, &data, &edata);
    writef("event_send %s %s\r\n", to.id, edata);
    bom_os_free(edata);
  }

  if (int result = read_message(&m))
    return result;
  if (m.code != CODE_SUCCESS)
    return m.code;
  
  string_to_event(m.data, event);
  return 0;
}

int bif::client::cmd_note_read(bom::note *note) {
  bif::message m;

  writef("note_read %s\r\n", note->id);

  if (int result = read_message(&m))
    return result;
  if (m.code != CODE_SUCCESS)
    return m.code;
  
  {
    char *string;

    if (hash_extract(m.data, "issuer", &string)) {
      note->_issuer = new bom::acct(string);
      bom_os_free(string);
    }
    if (hash_extract(m.data, "challenge", &string)) {
      note->_challenge = new dmath::bz;
      string_to_bz(string, note->_challenge);
      bom_os_free(string);
    }
    
    char *data = m.data;
    while (data = hash_extract(data, "value", &string)) {
      bom::value *v = new bom::value;
      string_to_value(string, v);
      v->next = note->_values;
      note->_values = v;
      bom_os_free(string);
    }
  }
  
  return 0;
}

int bif::client::cmd_note_withdraw(bom::value *vl, bom::note *note) {
  bif::message m;

  writef("note_withdraw");
  for (bom::value *v = vl; v; v = v->next) {
    char *sv;
    value_to_string(*v, &sv);
    writef(" %s", sv);
    bom_os_free(sv);
  }
  writef("\r\n");

  if (int result = read_message(&m))
    return result;
  if (m.code != CODE_SUCCESS)
    return m.code;
  
  note->set_id(m.data);
  return 0;
}

int bif::client::cmd_note_deposit(const bom::note &note,
 const dmath::bz &signature, bom::value **vl) {
  bif::message m;

  {
    char *ssignature; 
    bz_to_string(signature, &ssignature);
    writef("note_deposit %s %s\r\n", note.id, ssignature);
    bom_os_free(ssignature);
  }

  if (int result = read_message(&m))
    return result;
  if (m.code != CODE_SUCCESS)
    return m.code;

  char *p = m.data, *q;
  while (q = strchr(p, '\n')) {
    *q++ = '\0';
    bom::value *v = new bom::value;
    string_to_value(p, v);
    v->next = *vl;
    *vl = v;
    p = q;
  }

  return 0;
}

int bif::client::cmd_txn_create(bom::tid *txn) {
  bif::message m;

  writef("txn_create\r\n");

  if (int result = read_message(&m))
    return result;
  if (m.code != CODE_SUCCESS)
    return m.code;
  
  txn->set_id(m.data);
  return 0;
}

int bif::client::cmd_txn_commit() {
  bif::message m;

  writef("txn_commit\r\n");

  if (int result = read_message(&m))
    return result;
  if (m.code != CODE_SUCCESS)
    return m.code;
  
  return 0;
}

int bif::client::cmd_txn_abort() {
  bif::message m;

  writef("txn_abort\r\n");

  if (int result = read_message(&m))
    return result;
  if (m.code != CODE_SUCCESS)
    return m.code;
  
  return 0;
}
