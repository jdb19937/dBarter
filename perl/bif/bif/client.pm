package bif::client; 

use dmath;
use bom;
use bom::common;
use bif::common;

use IO::Socket;
use Exporter;

@ISA = qw(Exporter);

@EXPORT = qw(
  CODE_WELCOME
  CODE_ERR_USAGE
  CODE_ERR_UNKCOMMAND
  CODE_ERR_LINETOOLONG
  CODE_ERR_NOAUTH
  CODE_ERR_DB
  CODE_ERR_INVALID
  CODE_ERR_INSUFFICIENT
  CODE_ERR_NET
  CODE_ERR_EXISTS
  CODE_ERR_NOTFOUND
  CODE_ERR_ABORTED
  CODE_EVENT
  CODE_SUCCESS
  CODE_GOODBYE
);

sub CODE_WELCOME() { 100; }
sub CODE_ERR_USAGE() { 333; }
sub CODE_ERR_UNKCOMMAND() { 334; }
sub CODE_ERR_LINETOOLONG() { 338; }
sub CODE_ERR_NOAUTH() { 339; }
sub CODE_ERR_DB() { 340; }
sub CODE_ERR_INVALID() { 341; }
sub CODE_ERR_INSUFFICIENT() { 343; }
sub CODE_ERR_NET() { 352; }
sub CODE_ERR_EXISTS() { 358; }
sub CODE_ERR_NOTFOUND() { 359; }
sub CODE_ERR_ABORTED() { 360; }
sub CODE_EVENT() {  400; }
sub CODE_SUCCESS() { 500; }
sub CODE_GOODBYE() { 923; }

sub new($;$) {
  my ($class, $site) = @_;

  if ($site) {
    (bless { 'id' => $site, 'queue' => [ ] }, $class)->connect;
  } else {
    bless { }, $class;
  }
}

sub id($;$) {
  my ($self, $value) = @_;

  if (defined($value)) {
    $self->{'id'} = $value;
  } else {
    $self->{'id'};
  }
}

sub acct($;$) {
  my ($self, $value) = @_;

  if (defined($value)) {
    $self->{'acct'} = $value;
  } else {
    $self->{'acct'};
  }
}

sub connect($) {
  my $self = shift;

  return 0 if $self->{'socket'};
  my $site = $self->id;
 
  $site =~ /(^[^:]*)(?::(.*$))?/ or throw "cannot connect to host";
  my $host = $1;
  my $port = $2 || 1134;
  
  $self->{'socket'} = new IO::Socket::INET
    'PeerAddr' => $host,
    'PeerPort' => $port,
    'Proto' => 'tcp',
    'Type' => SOCK_STREAM;
  if (!$self->{'socket'}) {
    undef $self->{'socket'};
    throw "cannot connect to host";
  }
    
  my $message = $self->read;
  $self->id($message->{'data'});

  $self;
}

sub disconnect($) {
  my $self = shift;

  if ($self->{'socket'}) {
    $self->{'socket'}->close;
    undef $self->{'socket'};
  }
}

sub write($@) {
  my ($self, @args) = @_;

  my $socket = $self->{'socket'} or throw "not connected";
  print $socket join(" ", map { encode_string("$_") } @args) . "\r\n";
}

sub message_to_event($$) {
  my ($message, $event) = @_;

  my $hash = hash_from_string($$message{'data'});

  $event->id($$hash{'id'});
  $event->type($$hash{'type'});

  delete $$hash{'id'};
  delete $$hash{'type'};

  my $data = hash_to_string($hash);
  $event->data($data || "");

  $event;
}

sub read_one($) {
  my $self = shift;
  
  my $socket = $self->{'socket'} or throw "not connected";
  my ($code, $desc) = (<$socket> =~ /^([0-9]+) +(.*)/);
  
  my @data;
  while (<$socket>) {
    last if ($_ eq ".\n");
    push(@data, $_);
  }

  my $data = join '', @data;
  chomp($data);

  my $message = {
    'code' => $code,
    'desc' => $desc,
    'data' => $data
  };

  if ($$message{'code'} == 400) {
    my $event = new bom::event;
    message_to_event($message, $event);
    push(@{$$self{'queue'}}, $event);
  }

  $message;
}

sub read($;$) {
  my ($self) = @_;
  my $message;

  my %good_code = ( 500 => 1, 100 => 1 );
  
  $message = $self->read_one; 
  while ($$message{'code'} == 400) {
    $message = $self->read_one; 
  }
  
  throw $$message{'code'} if (!$good_code{$$message{'code'}});

  $message;
}


sub connected($) {
  shift->{'socket'} ? 1 : 0;
}

sub acct_create {
  my ($self, $acct) = @_;
  $self->write('acct_create', $acct->id, $acct->public, $acct->modulus);
  my $message = $self->read;
}

sub authenticated($) {
  shift->{'authenticated'} ? 1 : 0;
}

sub login($$) {
  my ($self, $acct) = @_;

  $self->write('login', $acct->id);
  my $message = $self->read;

  new dmath::bz $$message{'data'};
}

sub respond($$) {
  my ($self, $response) = @_;

  $self->write('respond', $response);
  my $message = $self->read;

  $$self{'authenticated'} = 1;
}

sub authenticate($$) {
  my ($self, $acct) = @_;
  
  my $challenge = $self->login($acct);

  my $modulus = $acct->modulus;
  my $secret = $acct->secret;
  my $response = rsa_crypt($challenge, $secret, $modulus);
  
  $self->respond($response);

  $$self{'acct'} = bless { %$acct }, bom::acct;
}

sub note_deposit($$) {
  my ($self, $note) = @_;
  
  my $signature = $note->signature;
  $self->write('note_deposit', $note->id, $signature);

  my $message = $self->read;
  my $portfolio = new_from_string bom::portfolio $$message{'data'};
}

sub note_list($) {
  my $self = shift;

  $self->write('note_list');
  my $message = $self->read;
  
  map { new bom::note $_ } split(/\s+/, $$message{'data'});
}
  
sub balance($) {
  my $self = shift;
  $self->write('balance');
  my $message = $self->read;
  
  my $portfolio = new_from_string bom::portfolio $$message{'data'};
}

sub credit($$$) {
  my ($self, $user, $value) = @_;
  $self->write('credit', $user->id, $value);
  my $message = $self->read;
  
  $value;
}

sub debit($$$) {
  my ($self, $user, $value) = @_;
  $self->write('debit', $user->id, $value);
  my $message = $self->read;
  
  $value;
}

sub note_read($$) {
  my ($self, $note) = @_;

  $self->write('note_read', $note->id);
  my $message = $self->read;

  my $hash = hash_from_string($$message{'data'});

  $note->issuer(new bom::acct $$hash{'issuer'});
  $note->challenge(new dmath::bz $$hash{'challenge'});
  my $portfolio = new_from_values bom::portfolio
    map { new_from_string bom::value $_ } array($$hash{'value'});
  $note->portfolio($portfolio);

  $note;
}

sub note_withdraw($@) {
  my ($self, $portfolio) = @_;

  $self->write('note_withdraw', $portfolio->to_string_list);
  my $message = $self->read;

  my $noteid = $message->{'data'};
  
  my $note = new bom::note;
  $note->id($noteid);
  $note->portfolio($portfolio);

  $note;
}

sub denom_create($$) {
  my ($self, $denom) = @_;

  $self->write('denom_create', $denom->id);
  my $message = $self->read;

  1;
}

sub denom_set($$$$) {
  my ($self, $denom, $attr, $value) = @_;

  my $string = {
    'flags' => sub {
      shift
    },
    'promise' => sub {
      shift;
    },
    'issuer' => sub {
      shift->id
    }
  }->{$attr}->($value);

  $self->write('denom_set', $denom->id, $attr, $string);
  my $message = $self->read;

  1;
}

sub denom_read($$) {
  my ($self, $denom) = @_;

  $self->write('denom_read', $denom->id);
  my $message = $self->read;
  
  my $hash = hash_from_string($$message{'data'});

  $denom->issuer(new bom::acct $$hash{'issuer'});
  $denom->promise($$hash{'promise'});
  $denom->flags($$hash{'flags'});

  $denom;
}

sub offer_list($) {
  my $self = shift;

  $self->write('offer_list');
  my $message = $self->read;
  
  map {
    new bom::offer $_
  } split(/\n/, $$message{'data'});
}

sub offer_create($) {
  my ($self, $ask, $bid) = @_;

  $self->write('offer_create', $ask, $bid);
  my $message = $self->read;
  
  new bom::offer $$message{'data'};
}

sub offer_destroy($) {
  my ($self, $offer) = @_;

  $self->write('offer_destroy', $offer->id);
  my $message = $self->read;

  1;
}

sub offer_read($$) {
  my ($self, $offer) = @_;

  $self->write('offer_read', $offer->id);
  my $message = $self->read;

  my $hash = hash_from_string($$message{'data'});
  
  $offer->bid_value(new_from_string bom::value $$hash{'bid'});
  $offer->ask_value(new_from_string bom::value $$hash{'ask'});
  $offer->issuer(new bom::acct $$hash{'issuer'}) if defined($$hash{'issuer'});

  $offer;
}

sub offer_list_by_denom($$$) {
  my ($self, $ask, $bid) = @_;

  $self->write('offer_list_by_denom', $ask->id, $bid->id);
  my $message = $self->read;

  map {
    new bom::offer $_
  } split(/\n/, $$message{'data'});
}

sub denom_list($) {
  my $self = shift;

  $self->write('denom_list');
  my $message = $self->read;
  
  map {
    new bom::denom $_
  } split(/\n/, $$message{'data'});
}

sub denom_list_all($) {
  my $self = shift;

  $self->write('denom_list_all');
  my $message = $self->read;
  
  map {
    new bom::denom $_
  } split(/\n/, $$message{'data'});
}

sub denom_search($$) {
  my $self = shift;
  my $pattern = shift;

  $self->write('denom_search', $pattern);
  my $message = $self->read;
  
  map {
    new bom::denom $_
  } split(/\n/, $$message{'data'});
}

sub acct_read($$) {
  my ($self, $acct) = @_;

  $self->write('acct_read', $acct->id);
  my $message = $self->read;

  my $hash = hash_from_string($$message{'data'});
  
  $acct->public(new dmath::bz $$hash{'public'});
  $acct->modulus(new dmath::bz $$hash{'modulus'});
  $acct->flags($$hash{'flags'});

  $acct;
}

sub txn_create($) {
  my ($self) = @_;
  
  $self->write('txn_create');
  my $message = $self->read;
  
  1;
}

sub txn_abort($) {
  my ($self) = @_;
  
  $self->write('txn_abort');
  my $message = $self->read;
  
  1;
}

sub txn_commit($) {
  my ($self) = @_;
  
  $self->write('txn_commit');
  my $message = $self->read;
  
  1;
}

sub event_read($$) {
  my ($self, $event) = @_;

  $self->write('event_read', $event->id);
  my $message = $self->read;

  message_to_event($message, $event);

  1;
}

sub event_list($;$;$) {
  my ($self, $lb, $ub) = @_;

  if (defined($ub)) {
    $self->write('event_list', $lb, $ub);
  } elsif (defined($lb)) {
    $self->write('event_list', $lb);
  } else {
    $self->write('event_list');
  }

  my $message = $self->read;
  map { new bom::event $_ } split(/\n/, $$message{'data'});
}

sub event_send($$$) {
  my ($self, $to, $data) = @_;

  $self->write('event_send', $to->id, $data);
  my $message = $self->read;
  
  $$message{'data'};
}

sub event_last($) {
  my ($self, $event, $lb, $ub) = @_;

  $self->write('event_last');

  my $message = $self->read;
  new bom::event $$message{'data'};
}

sub proxy_connect($$) {
  my ($self, $host) = @_;
  $self->write('connect', $host);
  my $message = $self->read;
  
  $$message{'data'};
}

sub proxy_reconnect($$) {
  my ($self, $sessionid) = @_;
  $self->write('reconnect', $sessionid);
  my $message = $self->read;
  
  1;
}

sub proxy_disconnect($) {
  my ($self) = @_;
  $self->write('disconnect');
  my $message = $self->read;
  
  1;
}

sub transaction($&) {
  my ($self, $code) = @_;
  my $status = CODE_ABORTED;
  my $active;

  do {
    my $redo = 0;

    $self->txn_create;
    $active = 1;
    
    try {
      &$code;
    } catch {
      $status = $_;
      $active = 0;
      if ($status == CODE_ERR_ABORTED) {;
        $redo = 1;
      } else {
        $self->txn_abort;
      }
    }
  } while ($redo);
  
  if ($active) {
    $self->txn_commit;
    0;
  } else {
    $status;
  }
}

1;
