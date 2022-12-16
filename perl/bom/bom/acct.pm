package bom::acct; 

use dmath;
use bom::object;
use bom::common;
use bom::txn;

@ISA = qw(bom::object);

sub public($;$) {
  my ($self, $public) = @_;
  if (defined($public)) {
    $self->{'public'} = $public;
  } else {
    $self->{'public'};
  }
}

sub secret($;$) {
  my ($self, $secret) = @_;
  if (defined($secret)) {
    $self->{'secret'} = $secret;
  } else {
    $self->{'secret'};
  }
}

sub modulus($;$) {
  my ($self, $modulus) = @_;
  if (defined($modulus)) {
    $self->{'modulus'} = $modulus;
  } else {
    $self->{'modulus'};
  }
}

sub passwd($;$) {
  my ($self, $passwd) = @_;
  if (defined($passwd)) {
    $self->{'passwd'} = $passwd;
  } else {
    $self->{'passwd'};
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

sub promise($;$) {
  my ($self, $promise) = @_;
  if (defined($promise)) {
    $self->{'promise'} = $promise;
  } else {
    $self->{'promise'};
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

sub note_list($;$) {
  my ($self, $note_list) = @_;
  if (defined($note_list)) {
    $self->{'note_list'} = $note_list;
  } else {
    $self->{'note_list'};
  }
}

sub denom_list($;$) {
  my ($self, $denom_list) = @_;
  if (defined($denom_list)) {
    $self->{'denom_list'} = $denom_list;
  } else {
    $self->{'denom_list'};
  }
}

sub event_list($;$) {
  my ($self, $event_list) = @_;
  if (defined($event_list)) {
    $self->{'event_list'} = $event_list;
  } else {
    $self->{'event_list'};
  }
}

sub get_acct_list($;$) {
  my ($class, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $sul;
  my $result = bom::_get_acct_list($$txn, $sul);
  $result and throw $result;
  [ map { new bom::acct $_ } split(/\s+/, $sul) ];
}

sub exists($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $exists;
  my $result = bom::_acct_exists($self->id, $$txn, $exists);
  $result and throw $result;
  ord($exists);
}

sub get_passwd($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  bom::bom_db_get("acct_passwd.db", $self->id, $txn);
}

sub set_passwd($$;$) {
  my ($self, $passwd, $txn) = @_;
  $txn ||= $bom::txn::default;
  bom::bom_db_put("acct_passwd.db", $self->id, $passwd, $txn);
}

sub del_passwd($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  bom::bom_db_del("acct_passwd.db", $self->id, $txn);
}

sub get_promise($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $promise;
  my $result = bom::_acct_get_promise($self->id, $$txn, $promise);
  $result and throw $result;
  $promise;
}

sub set_promise($$;$) {
  my ($self, $promise, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_acct_set_promise($self->id, $$txn, $promise);
  $result and throw $result;
}

sub del_promise($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_acct_del_promise($self->id, $$txn);
  $result and throw $result;
}


sub get_flags($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $flags;
  my $result = bom::_acct_get_flags($self->id, $$txn, $flags);
  $result and throw $result;
  $flags;
}

sub set_flags($$;$) {
  my ($self, $flags, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_acct_set_flags($self->id, $$txn, $flags);
  $result and throw $result;
}

sub del_flags($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_acct_del_flags($self->id, $$txn);
  $result and throw $result;
}


sub get_secret($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $ssecret;
  my $result = bom::_acct_get_secret($self->id, $$txn, $ssecret);
  $result && $result != DB_NOTFOUND and throw $result;
  $result ? undef : new dmath::bz $ssecret;
}

sub set_secret($$;$) {
  my ($self, $secret, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_acct_set_secret($self->id,
   $$txn, $secret->to_string);
  $result and throw $result;
}

sub del_secret($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_acct_del_secret($self->id, $$txn);
  $result and throw $result;
}

sub get_modulus($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $smodulus;
  my $result = bom::_acct_get_modulus($self->id, $$txn, $smodulus);
  $result and throw $result;
  new dmath::bz $smodulus;
}

sub set_modulus($$;$) {
  my ($self, $modulus, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_acct_set_modulus($self->id,
   $$txn, $modulus->to_string);
  $result and throw $result;
}

sub del_modulus($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_acct_del_modulus($self->id, $$txn);
  $result and throw $result;
}

sub get_public($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $spublic;
  my $result = bom::_acct_get_public($self->id, $$txn, $spublic);
  $result and throw $result;
  new dmath::bz $spublic;
}

sub set_public($$;$) {
  my ($self, $public, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_acct_set_public($self->id,
   $$txn, $public->to_string);
  $result and throw $result;
}

sub del_public($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_acct_del_public($self->id, $$txn);
  $result and throw $result;
}


sub get_portfolio($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $sportfolio;
  my $result = bom::_acct_get_portfolio($self->id, $$txn, $sportfolio);
  $result and throw $result;
  new_from_string bom::portfolio $sportfolio;
}

sub set_portfolio($$;$) {
  my ($self, $portfolio, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_acct_set_portfolio($self->id,
   $$txn, $portfolio->to_string);
  $result and throw $result;
}

sub del_portfolio($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_acct_del_portfolio($self->id, $$txn);
  $result and throw $result;
}


sub get_note_list($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $snote_list;
  my $result = bom::_acct_get_note_list($self->id, $$txn, $snote_list);
  $result and throw $result;
  [ map { new bom::note $_ } split(/\s+/, $snote_list) ];
}

sub del_note_list($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_acct_del_note_list($self->id, $$txn);
  $result and throw $result;
}

sub get_denom_list($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $sdenom_list;
  my $result = bom::_acct_get_denom_list($self->id, $$txn, $sdenom_list);
  $result and throw $result;
  [ map { new bom::denom $_ } split(/\s+/, $sdenom_list) ];
}

sub get_event_list($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $sevent_list;
  my $result = bom::_acct_get_event_list($self->id, $$txn, $sevent_list);
  $result and throw $result;
  [ map { new bom::event $_ } split(/\s+/, $sevent_list) ];
}

sub get_offer_list($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $soffer_list;
  my $result = bom::_acct_get_offer_list($self->id, $$txn, $soffer_list);
  $result and throw $result;
  [ map { new bom::offer $_ } split(/\s+/, $soffer_list) ];
}

sub read($;$) {
  my ($self, $txn) = @_;
  $$self{'public'} = $self->get_public($txn);
  $$self{'secret'} = $self->get_secret($txn);
  $$self{'modulus'} = $self->get_modulus($txn);
  $$self{'flags'} = $self->get_flags($txn);
  $$self{'promise'} = $self->get_promise($txn);
  try {
    $$self{'passwd'} = $self->get_passwd($txn);
  } catch {
    throw $_ unless $_ == '-7';
  };
  $self;
}

sub write($;$) {
  my ($self, $txn) = @_;
  $self->set_public($$self{'public'}, $txn) if defined($$self{'public'});
  $self->set_secret($$self{'secret'}, $txn) if defined($$self{'secret'});
  $self->set_modulus($$self{'modulus'}, $txn) if defined($$self{'modulus'});
  $self->set_flags($$self{'flags'}, $txn) if defined($$self{'flags'});
  $self->set_promise($$self{'promise'}, $txn) if defined($$self{'promise'});
  $self->set_passwd($$self{'passwd'}, $txn) if defined($$self{'passwd'});
  $self;
}

sub destroy($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_acct_destroy($self->id, $$txn);
  $result and throw $result;
}

sub note_sign($$) {
  my ($self, $note) = @_;

  my $challenge = $note->challenge;
  my $secret = $self->secret;
  my $modulus = $self->modulus;
  my $signature = rsa_crypt($challenge, $secret, $modulus);
  
  $note->signature($signature);
  $note;
}

1;
