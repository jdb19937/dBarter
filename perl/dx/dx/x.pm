package dx::x;
use dx::log;
use dx::object;

use UNIVERSAL qw(isa can);

sub new {
  my ($pkg, %param) = @_;
  ref($pkg) and die "dx::x::new invalid 0\n";
  
  bless {
    'id' => undef,
    'log' => undef,
    'status' => '',
    'waiting_r' => { },
    'waiting_rw' => { },
    'asset_r' => { },
    'asset_rw' => { },
    %param
  }, $pkg;
}

sub DESTROY {
  my ($x) = @_;
  $$x{'status'} eq 'open' and $x->abort;
}

sub open {
  my ($x) = @_;
  ref($x) or die "dx::x::open invalid 0\n";
  $$x{'status'} and die "dx::x::open invalid 1\n";

  $$x{'log'}->lock_writer;
  $$x{'log'}->sync_reader;

  $$x{'id'} = ${$$x{'log'}}{'x_next'}++;
  ${${$$x{'log'}}{'x'}}{$$x{'id'}} = $x;
  $$x{'log'}->write($$x{'id'}, 'open');

  $$x{'log'}->sync_reader;
  $$x{'log'}->unlock_writer;
  
  $$x{'status'} eq 'open' or die "dx::x::open error 2\n";
  0;
}

sub abort {
  my ($x) = @_;
  ref($x) or die "dx::x::abort invalid 0\n";

  $$x{'log'}->lock_writer;
  $$x{'log'}->sync_reader;

  if ($$x{'status'} ne 'open') {
    $$x{'log'}->unlock_writer;
    return 'x-not-open';
  }

  $$x{'log'}->write($$x{'id'}, 'abort');
  $$x{'log'}->sync_reader;
  $$x{'log'}->unlock_writer;
  
  $$x{'status'} eq 'aborted' or die "dx::x::abort error 1\n";
  0;
}

sub commit {
  my ($x) = @_;
  ref($x) or die "dx::x::commit invalid 0\n";

  $$x{'log'}->lock_writer;
  $$x{'log'}->sync_reader;

  if ($$x{'status'} ne 'open') {
    $$x{'log'}->unlock_writer;
    return 'x-not-open';
  }

  $$x{'log'}->write($$x{'id'}, 'commit');
  $$x{'log'}->sync_reader;
  $$x{'log'}->unlock_writer;
  
  $$x{'status'} eq 'committed' or die "dx::x::abort error 1\n";
  0;
}

sub lock_r {
  my ($x, $object) = @_;
  my $object = new dx::object 'id' => $object_id, $log => $$x{'log'};

  $$x{'log'}->sync_reader;
  $object->read;

again:
  while (keys(%{$$object{'owner_rw'}})) {
    $$x{'log'}->sync_reader_one;
    $object->read;
  }

  $$x{'log'}->lock_writer;
  $$x{'log'}->sync_reader;
  $object->read;

  if (keys(%{$$object{'owner_rw'}})) {
    $$x{'log'}->unlock_writer;
    goto again;
  }

  $$x{'log'}->write($$x{'id'}, 'lock_r', $$object{'id'});
  $$x{'log'}->sync_reader;
  $object->read;

  $$x{'log'}->unlock_writer;
  0;
}

sub lock_rw {
  my ($x, $object_id) = @_;
  my $object = new dx::object 'id' => $object_id, $log => $$x{'log'};

  $$x{'log'}->sync_reader;
  $object->read;

again:
  while (keys(%{$$object{'owner_r'}}) || keys(%{$$object{'owner_rw'}})) {
    $$x{'log'}->sync_reader_one;
    $object->read;
  }

  $$x{'log'}->lock_writer;
  $$x{'log'}->sync_reader;
  $object->read;

  if (keys(%{$$object{'owner_r'}}) || keys(%{$$object{'owner_rw'}})) {
    $$x{'log'}->unlock_writer;
    goto again;
  }

  $$x{'log'}->write($$x{'id'}, 'lock_rw', $$object{'id'});
  $$x{'log'}->sync_reader;
  $object->read;

  $$x{'log'}->unlock_writer;
  0;
}

sub unlock {
  my ($x, $object_id) = @_;
  my $object = new dx::object 'id' => $object_id, $log => $$x{'log'};

  $$x{'log'}->lock_writer;
  $$x{'log'}->sync_reader;
  $object->read;

  ${$$object{'owner_r'}}{$$x{'id'}} || ${$$object{'owner_rw'}}{$$x{'id'}} or
   die "dx::x::unlock invalid 0\n";

  $$x{'log'}->write($$x{'id'}, 'unlock', $$object{'id'});
  $$x{'log'}->sync_reader;
  $object->read;

  $$x{'log'}->unlock_writer;
  0;
}

1;
