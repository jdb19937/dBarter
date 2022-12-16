package bom::event;
use bom::common;
use bom::txn;
use bom::object;

@ISA = qw(bom::object);

#sub AUTOLOAD {
#  my ($self, $value) = @_;
#  my $key = $AUTOLOAD;
#  $key =~ s/^bom::event:://;
#  my $hash = hash_from_string($$self{'data'});
#  if (defined($value)) {
#    $$hash{$key} = $value;
#  }
#  $$self{'data'} = hash_to_string($hash);
#  $$hash{$key};
#}
#
#sub DESTROY {
#
#}

#sub acct_list($;$) {
#  my ($self, $acct_list) = @_;
#  if (defined($acct_list)) {
#    $self->{'acct_list'} = $acct_list;
#  } else {
#    $self->{'acct_list'};
#  }
#}

sub data($;$) {
  my ($self, $data) = @_;
  if (defined($data)) {
    $self->{'data'} = $data;
  } else {
    $self->{'data'};
  }
}

sub type($;$) {
  my ($self, $type) = @_;
  if (defined($type)) {
    $self->{'type'} = $type;
  } else {
    $self->{'type'};
  }
}

sub get_last_event($;$) {
  my ($class, $txn) = @_;
  my $sevent;
  my $result = bom::_get_last_event($$txn, $sevent);
  $result && $result != DB_NOTFOUND and throw $result;
  $result ? bom::event->new("0") : bom::event->new($sevent);
}

sub exists($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $exists;
  my $result = bom::_event_exists($self->id, $$txn, $exists);
  $result and throw $result;
  ord($exists);
}

sub get_type($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $type;
  my $result = bom::_event_get_type($self->id, $$txn, $type);
  $result and throw $result;
  $type;
}

sub set_type($$;$) {
  my ($self, $type, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_event_set_type($self->id, $$txn, $type);
  $result and throw $result;
}

sub del_type($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_event_del_type($self->id, $$txn);
  $result and throw $result;
}

sub get_data($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $data;
  my $result = bom::_event_get_data($self->id, $$txn, $data);
  $result and throw $result;
  $data;
}

sub set_data($$;$) {
  my ($self, $data, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_event_set_data($self->id, $$txn, $data);
  $result and throw $result;
}

sub del_data($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_event_del_data($self->id, $$txn);
  $result and throw $result;
}


#sub get_acct_list($;$) {
#  my ($self, $txn) = @_;
#  $txn ||= $bom::txn::default;
#  my $sacct_list;
#  my $result = bom::_event_get_acct_list($self->id, $$txn, $sacct_list);
#  $result and throw $result;
#  [ map { new bom::acct $_ } split(/\s+/, $sacct_list) ];
#}
#
#sub set_acct_list($$;$) {
#  my ($self, $acct_list, $txn) = @_;
#  $txn ||= $bom::txn::default;
#  my $sacct_list = join(" ", map { $_->id } @$acct_list);
#  my $result = bom::_event_set_acct_list($self->id, $$txn, $sacct_list);
#  $result and throw $result;
#}
#
#sub del_acct_list($;$) {
#  my ($self, $txn) = @_;
#  $txn ||= $bom::txn::default;
#  my $result = bom::_event_del_acct_list($self->id, $$txn);
#  $result and throw $result;
#}

sub read($;$) {
  my ($self, $txn) = @_;
  $$self{'data'} = $self->get_data($txn);
#  $$self{'acct_list'} = $self->get_acct_list($txn);
  $$self{'type'} = $self->get_type($txn);
  $self;
}

sub write($;$) {
  my ($self, $txn) = @_;
  $self->set_data($$self{'data'}, $txn) if defined($$self{'data'});
  $self->set_type($$self{'type'}, $txn) if defined($$self{'type'});
#  $self->set_acct_list($$self{'acct_list'}, $txn)
#   if defined($$self{'acct_list'});
  $self;
}

sub destroy($;$) {
  my ($self, $txn) = @_;
  $txn ||= $bom::txn::default;
  my $result = bom::_event_destroy($self->id, $$txn);
  $result and throw $result;
}

1;
