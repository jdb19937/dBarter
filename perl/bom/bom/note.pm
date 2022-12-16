package bom::note; 

use dmath;
use bom::object;
use bom::txn;
use bom::acct;
use bom::portfolio;

@ISA = qw(bom::object);

sub gen_id($) {
  my $self = shift;
  $self->id(sprintf("NOTE:%08X%08X", time, rand));
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

sub portfolio($;$) {
  my ($self, $portfolio) = @_;
  if (defined($portfolio)) {
    $self->{'portfolio'} = $portfolio;
  } else {
    $self->{'portfolio'};
  }
}

sub challenge($;$) {
  my ($self, $challenge) = @_;
  if (defined($challenge)) {
    $self->{'challenge'} = $challenge;
  } else {
    $self->{'challenge'};
  }
}

sub signature($;$) {
  my ($self, $signature) = @_;
  if (defined($signature)) {
    $self->{'signature'} = $signature;
  } else {
    $self->{'signature'};
  }
}

sub exists($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $exists;
  my $result = bom::_note_exists($self->id, $$txn, $exists);
  $result and throw $result;
  ord($exists);
}

sub get_issuer($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $issuer = new bom::acct;
  my $result = bom::_note_get_issuer($self->id, $$txn, $$issuer{'id'});
  $result and throw $result;
  $issuer;
}

sub set_issuer($$;$) {
  my ($self, $issuer, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_note_set_issuer($self->id, $$txn, $issuer->id);
  $result and throw $result;
}

sub del_issuer($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_note_del_issuer($self->id, $$txn);
  $result and throw $result;
}

sub get_portfolio($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $sportfolio;
  my $result = bom::_note_get_portfolio($self->id, $$txn, $sportfolio);
  $result and throw $result;
  new_from_string bom::portfolio $sportfolio;
}

sub set_portfolio($$;$) {
  my ($self, $portfolio, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_note_set_portfolio($self->id,
   $$txn, $portfolio->to_string);
  $result and throw $result;
}

sub del_portfolio($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_note_del_portfolio($self->id, $$txn);
  $result and throw $result;
}

sub get_challenge($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $schallenge;
  my $result = bom::_note_get_challenge($self->id, $$txn, $schallenge);
  $result and throw $result;
  new dmath::bz $schallenge;
}

sub set_challenge($$;$) {
  my ($self, $challenge, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_note_set_challenge($self->id,
   $$txn, $challenge->to_string);
  $result and throw $result;
}

sub del_challenge($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_note_del_challenge($self->id, $$txn);
  $result and throw $result;
}

sub read($;$) {
  my ($self, $txn) = @_;
  $$self{'challenge'} = $self->get_challenge($txn);
  $$self{'issuer'} = $self->get_issuer($txn);
  $$self{'portfolio'} = $self->get_portfolio($txn);
  $self;
}

sub write($;$) {
  my ($self, $txn) = @_;
  $self->set_challenge($$self{'challenge'}, $txn)
   if defined($$self{'challenge'});
  $self->set_issuer($$self{'issuer'}, $txn) if defined($$self{'issuer'});
  $self->set_portfolio($$self{'portfolio'}, $txn)
   if defined($$self{'portfolio'});
  $self;
}

sub destroy($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_note_destroy($self->id, $$txn);
  $result and throw $result;
}

1;
