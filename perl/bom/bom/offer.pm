package bom::offer; 

use dmath;
use bom::object;
use bom::txn;
use bom::acct;
use bom::value;
use bom::portfolio;

@ISA = qw(bom::object);

sub gen_id($) {
  my $self = shift;
  $self->id(sprintf("OFFER:%08X%08X", time, rand));
  $self;
}

sub issuer($;$) {
  my ($self, $issuer) = @_;
  if (defined($issuer)) {
    $self->{'issuer'} = $issuer;
  } else {
    $self->{'issuer'};
  }
}

sub ask_value($;$) {
  my ($self, $ask_value) = @_;
  if (defined($ask_value)) {
    $self->{'ask_value'} = $ask_value;
  } else {
    $self->{'ask_value'};
  }
}

sub bid_value($;$) {
  my ($self, $bid_value) = @_;
  if (defined($bid_value)) {
    $self->{'bid_value'} = $bid_value;
  } else {
    $self->{'bid_value'};
  }
}

sub exists($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $exists;
  my $result = bom::_offer_exists($self->id, $$txn, $exists);
  $result and throw $result;
  ord($exists);
}

sub get_issuer($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $issuer = new bom::acct;
  my $result = bom::_offer_get_issuer($self->id, $$txn, $$issuer{'id'});
  $result and throw $result;
  $issuer;
}

sub set_issuer($$;$) {
  my ($self, $issuer, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_offer_set_issuer($self->id, $$txn, $issuer->id);
  $result and throw $result;
}

sub del_issuer($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_offer_del_issuer($self->id, $$txn);
  $result and throw $result;
}

sub get_ask_value($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $sask_value;
  my $result = bom::_offer_get_ask_value($self->id, $$txn, $sask_value);
  $result and throw $result;
  new_from_string bom::value $sask_value;
}

sub set_ask_value($$;$) {
  my ($self, $ask_value, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_offer_set_ask_value($self->id,
   $$txn, $ask_value->to_string);
  $result and throw $result;
}

sub del_ask_value($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_offer_del_ask_value($self->id, $$txn);
  $result and throw $result;
}

sub get_bid_value($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $sbid_value;
  my $result = bom::_offer_get_bid_value($self->id, $$txn, $sbid_value);
  $result and throw $result;
  new_from_string bom::value $sbid_value;
}

sub set_bid_value($$;$) {
  my ($self, $bid_value, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_offer_set_bid_value($self->id,
   $$txn, $bid_value->to_string);
  $result and throw $result;
}

sub del_bid_value($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_offer_del_bid_value($self->id, $$txn);
  $result and throw $result;
}

sub read($;$) {
  my ($self, $txn) = @_;
  $$self{'issuer'} = $self->get_issuer($txn);
  $$self{'ask_value'} = $self->get_ask_value($txn);
  $$self{'bid_value'} = $self->get_bid_value($txn);
  $self;
}

sub write($;$) {
  my ($self, $txn) = @_;
  $self->set_issuer($$self{'issuer'}, $txn) if defined($$self{'issuer'});
  $self->set_ask_value($$self{'ask_value'}, $txn)
   if defined($$self{'ask_value'});
  $self->set_bid_value($$self{'bid_value'}, $txn)
   if defined($$self{'bid_value'});
  $self;
}

sub destroy($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_offer_destroy($self->id, $$txn);
  $result and throw $result;
}

1;
