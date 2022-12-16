// {HEADER}

#define _MARKET_CC
#include "bif.h"

bif::market::market(bif::server *_sv) {
  txn_t *txn;
  int result;
  
  sv = _sv;
  denom_list = NULL;
  trader_list = NULL;

  if (0) {
again:
    if (result = txn_new(&txn)) {
      errorf("database error: %d\n", result);
      exit(1);
    }
    result = get_denom_list(txn, &denom_list);
    if (result == EAGAIN) {
      txn_abort(txn);
      goto again;
    }
    if (result) {
      txn_abort(txn);
      errorf("database error: %d\n", result);
      exit(1);
    }

    if (int result = txn_commit(txn)) {
      errorf("database error: %d\n", result);
      exit(1);
    }

//    for (bom::denom *d0 = denom_list; d0; d0 = d0->next) {
//      for (bom::denom *d1 = d0->next; d1; d1 = d1->next) {
//        bif::trader *trader = new bif::trader(this, *d0, *d1);
//      }
//    }
  }
}

bif::market::~market() {
  while (trader_list)
    delete trader_list;
  if (denom_list)
    delete denom_list;
}

int bif::market::signal(const bom::denom &d0, const bom::denom &d1) {
  bif::trader *trader;

  if (!(trader = new bif::trader(this, d0, d1)))
    return -1;
  
  return 0;
}
