// {HEADER}

int __ctype_b = 0; // libdb 2.7.5 fuxorz

#define _BARTERD_CC
#include <bom.h>
#include <bif.h>
#include <signal.h>
#include "barterd.h"

char *_db_root = bom_os_strdup("/usr/local/dbarter/db");
bif::site site("", 1134);

void usage() {
  fprintf(stderr, "Usage: barterd [-d db_directory] [-h host] [-p port]\n");
  exit(1);
}

void write_pid() {
  FILE *fp;
  
  if (chdir(_db_root) < 0) {
    perror(_db_root);
    exit(1);
  }

  if (fp = fopen("barterd.pid", "r")) {
    errorf("barterd: barterd.pid file exists; "
     "Already running or exited abnormally.\n");
    exit(1);
  }

  if (!(fp = fopen("barterd.pid", "w"))) {
    perror("barterd.pid");
    exit(1);
  }

  fprintf(fp, "%d\n", getpid());
  fclose(fp);
}

void death() {
  chdir(_db_root);
  unlink("barterd.pid");
  bom_db_exit();
  exit(0);
}

int main(int argc, char **argv) {
  bif::server *sv;
  
  for (int i = 1; i < argc; i++) {
    if (*argv[i] == '-') {
      switch (*(argv[i] + 1)) {
      case 'd':
	if (++i == argc)
	  usage();
	_db_root = bom_os_strdup(argv[i]);
	break;
      case 'h':
	if (++i == argc)
	  usage();
	site.set_host(argv[i]);
	break;
      case 'p':
	if (++i == argc)
	  usage();
	site.set_port(atoi(argv[i]));
	break;
      default:
	usage();
      }
    } else
      usage();
  }
  
  signal(SIGPIPE, SIG_IGN);

  if (chdir(_db_root)) {
    perror(_db_root);
    exit(1);
  }

  write_pid();

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  
  bom_db_init(_db_root, 1);
  bif_jump_set("shutdown", (void *)death);
  
  srand(time(0) + getpid());
  
  sv = new bif::server;
  sv->open(site.host, site.port);

  {
    txn_t *txn;
    int result;
    char exists;
    bom::acct root;
    
  again:
    if (int result = txn_new(&txn)) {
      errorf("cannot begin transaction\n");
      bom_db_exit();
      exit(1);
    }
    
    root.set_id("root");

    result = root.exists(txn, &exists);
    if (result == EAGAIN) {
      txn_abort(txn);
      goto again;
    } else if (result) {
      txn_abort(txn);
      errorf("error creating root\n");
      bom_db_exit();
      exit(1);
    }
    if (!exists) {
      root._modulus = new dmath::bz(2);
      root._public = new dmath::bz(1);
      root._promise = bom_os_strdup("");
      root._flags = ACCT_FLAG_SUPER;
      
      result = root.set(txn,
       ACCT_ATTR_PUBLIC | ACCT_ATTR_MODULUS |
       ACCT_ATTR_FLAGS | ACCT_ATTR_PROMISE);
      if (result == EAGAIN) {
        txn_abort(txn);
        goto again;
      } else if (result) {
        txn_abort(txn);
        errorf("error creating root\n");
        bom_db_exit();
        exit(1);
      }
    }

    result = txn_commit(txn);
    if (result == EAGAIN) {
      txn_abort(txn);
      goto again;
    } else if (result) {
      txn_abort(txn);
      errorf("error committing transaction\n");
      bom_db_exit();
      exit(1);
    }
  }

  while (1)
    sv->accept();
}

