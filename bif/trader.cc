// {HEADER}

#define _TRADER_CC
#include "bif.h"

void *_trader_thread(void *_trader) {
  bif::trader *trader = (bif::trader *)_trader;
  int retval;

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  
  if (int result = trader->process())
    errorf("database error: %d\n", result);
  delete trader;
}

bif::trader::trader(bif::market *_market,
 const bom::denom &d0, const bom::denom &d1) {

  market = _market;
  denom[0].set_id(d0.id);
  denom[1].set_id(d1.id);
  prev = NULL;
  next = NULL;
  
  if (market) {
    market->lock();
    link();
    market->unlock();
  }
  
  spawn(_trader_thread, this);
  detach();
}

bif::trader::~trader() {
  if (market) {
    market->lock();
    unlink();
    market->unlock();
  }
  cancel();
}

void bif::trader::link() {
  bif::trader **head;
 
  head = &market->trader_list; 
  prev = NULL;
  next = *head;
  if (*head)
    (*head)->prev = this;
  *head = this;
}

void bif::trader::unlink() {
  bif::trader **head;
 
  head = &market->trader_list; 
  if (prev)
    prev->next = next;
  if (next)
    next->prev = prev;
  if (this == *head)
    *head = next;
}

int bif::trader::execute(txn_t *txn, char *_success) {
  bom::acct issuer[2];
  dmath::bq ratio[2];
  bom::value bid[2], ask[2];
  bom::value credit[2], debit[2];
  bom::offer offer[2];
  bom::value cut;
  dmath::bq theta[2];
  int result;
  char exists, success;
  
  *_success = 0;

  if (result = find_best_offer(txn,
   denom[0], denom[1], &offer[0], &ratio[0], &exists)) 
    return result;
  if (!exists)
    return 0;

  if (result = find_best_offer(txn,
   denom[1], denom[0], &offer[1], &ratio[1], &exists)) 
    return result;
  if (!exists)
    return 0;
  
  if (ratio[0] * ratio[1] > 1)
    return 0;
  
  if (result = offer[0].get_bid_value(txn, &bid[0]))
    return result;
  if (result = offer[0].get_ask_value(txn, &ask[0]))
    return result;
  if (result = offer[0].get_issuer(txn, &issuer[0]))
    return result;
  if (result = offer[1].get_bid_value(txn, &bid[1]))
    return result;
  if (result = offer[1].get_ask_value(txn, &ask[1]))
    return result;
  if (result = offer[1].get_issuer(txn, &issuer[1]))
    return result;
  
  if (bid[0].quant < ask[1].quant) {
    theta[0] = 1;
    theta[1] = bid[0].quant / ask[1].quant;
  } else if (bid[1].quant < ask[0].quant) {
    theta[0] = bid[1].quant / ask[0].quant;
    theta[1] = 1;
  } else {
    theta[0] = 1;
    theta[1] = 1;
  }

  credit[0] = ask[0];
  credit[0].quant *= theta[0];
  debit[0] = bid[0];
  debit[0].quant *= theta[0];

  credit[1] = ask[1];
  credit[1].quant *= theta[1];
  debit[1] = bid[1];
  debit[1].quant *= theta[1];

  result = issuer[0].credit(txn, credit[0], &success);
  if (result)
    return result;
  if (!success)
    return -1;
  ask[0].quant -= credit[0].quant;
  result = offer[0].set_ask_value(txn, ask[0]);
  if (result)
    return result;

  result = issuer[1].credit(txn, credit[1], &success);
  if (result)
    return result;
  if (!success)
    return -1;
  ask[1].quant -= credit[1].quant;
  result = offer[1].set_ask_value(txn, ask[1]);
  if (result)
    return result;

//  cut.denom.set_id(debit[0].denom.id);
//  cut.quant = debit[0].quant - credit[1].quant;
//  result = market->acct.credit(txn, cut, &success);
//  if (result)
//    return result;
//  if (!success)
//    return -1;
  bid[0].quant -= debit[0].quant;
  if (bid[0].quant == 0) {
    if (result = offer[0].destroy(txn))
      return result;
  } else if (result = offer[0].set_bid_value(txn, bid[0]))
    return result;

//  cut.denom.set_id(debit[1].denom.id);
//  cut.quant = debit[1].quant - credit[0].quant;
//  result = market->acct.credit(txn, cut, &success);
//  if (result)
//    return result;
//  if (!success)
//    return -1;
  bid[1].quant -= debit[1].quant;
  if (bid[1].quant == 0) {
    if (result = offer[1].destroy(txn))
      return result;
  } else if (result = offer[1].set_bid_value(txn, bid[1]))
    return result;
  
  *_success = 1;

  return 0;
}

int bif::trader::process() {
  txn_t *txn;
  int result;
  char success = 0;

  if (!strcmp(denom[0].id, denom[1].id))
    return 0;

again:
  do {
    if (result = txn_new(&txn))
      return result;
    
    result = execute(txn, &success);
    if (result == EAGAIN) {
      txn_abort(txn);
      goto again;
    }
    if (result) {
      txn_abort(txn);
      return result;
    }
    if (!success) {
      txn_abort(txn);
      return 0;
    }

    txn_commit(txn);
  } while (1);
}

