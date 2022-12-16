package bom::denom;

use dmath;
use bom::object;
use bom::txn;

@ISA = qw(bom::object);

sub issuer($;$) {
  my ($self, $issuer) = @_;
  if (defined($issuer)) {
    $self->{'issuer'} = $issuer;
  } else {
    $self->{'issuer'};
  }
}

sub promise($;$) {
  my ($self, $promise) = @_;
  if (defined($promise)) {
    $self->{'promise'} = $promise;
  } else {
    $self->{'promise'};
  }
}

sub flags($;$) {
  my ($self, $flags) = @_;
  if (defined($flags)) {
    $self->{'flags'} = $flags;
  } else {
    $self->{'flags'};
  }
}

sub exists($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $exists;
  my $result = bom::_denom_exists($self->id, $$txn, $exists);
  $result and throw $result;
  ord($exists);
}

sub get_issuer($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $issuer = new bom::acct;
  my $result = bom::_denom_get_issuer($self->id, $$txn, $$issuer{'id'});
  $result and throw $result;
  $issuer;
}

sub set_issuer($$;$) {
  my ($self, $issuer, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_denom_set_issuer($self->id, $$txn, $issuer->id);
  $result and throw $result;
}

sub del_issuer($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_denom_del_issuer($self->id, $$txn);
  $result and throw $result;
}

sub get_promise($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $promise;
  my $result = bom::_denom_get_promise($self->id, $$txn, $promise);
  $result and throw $result;
  $promise;
}

sub set_promise($$;$) {
  my ($self, $promise, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_denom_set_promise($self->id, $$txn, $promise);
  $result and throw $result;
}

sub del_promise($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_denom_del_promise($self->id, $$txn);
  $result and throw $result;
}

sub get_flags($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $flags;
  my $result = bom::_denom_get_flags($self->id, $$txn, $flags);
  $result and throw $result;
  $flags;
}

sub set_flags($$;$) {
  my ($self, $flags, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_denom_set_flags($self->id, $$txn, $flags);
  $result and throw $result;
}

sub del_flags($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_denom_del_flags($self->id, $$txn);
  $result and throw $result;
}

sub read($;$) {
  my ($self, $txn) = @_;
  $$self{'issuer'} = $self->get_issuer($txn);
  $$self{'flags'} = $self->get_flags($txn);
  $$self{'promise'} = $self->get_promise($txn);
  $self;
}

sub write($;$) {
  my ($self, $txn) = @_;
  $self->set_issuer($$self{'issuer'}, $txn) if defined($$self{'issuer'});
  $self->set_flags($$self{'flags'}, $txn) if defined($$self{'flags'});
  $self->set_promise($$self{'promise'}, $txn) if defined($$self{'promise'});
  $self;
}

sub destroy($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_denom_destroy($self->id, $$txn);
  $result and throw $result;
}

1;
