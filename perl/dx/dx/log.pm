package dx::log;
use dx::x; 

use UNIVERSAL qw(isa can);
use IO::File;
use IO::Seekable;
use Fcntl qw(:flock);

%dx::log::default_op_table = (
  'open' => \&op_open,
  'abort' => \&op_abort,
  'commit' => \&op_commit,
  'lock_r' => \&op_lock_r,
  'lock_rw' => \&op_lock_rw,
  'unlock' => \&op_unlock,
);

sub new {
  my ($pkg, %param) = @_;
  
  ref($pkg) and die "dx::log::new invalid 0\n";
  my $log = bless {
    'path' => undef,
    'fhr' => undef,
    'fhw' => undef,
    'x' => { },
    'x_open' => { },
    'x_closed' => { },
    'x_next' => undef,
    'op_table' => { },
    'object' => { },
    %param,
  }, $pkg;
  
  $$log{'op_table'} = {
    %dx::log::default_op_table,
    %{$$log{'op_table'}}
  };
  
  $log
}

sub DESTROY {
  my ($log) = @_;
  $log->close;
}

sub open {
  my ($log) = @_;

  $$log{'path'} or die "dx::log::new invalid 1\n";

  $$log{'fhr'} = new IO::File $$log{'path'},
   O_RDONLY | O_CREAT | O_SYNC;
  $$log{'fhw'} = new IO::File $$log{'path'},
   O_RDWR | O_CREAT | O_APPEND | O_SYNC;
   
  0;
}

sub close {
  my ($log) = @_;
  
  if ($$log{'fhr'}) {
    $$log{'fhr'}->close;
    undef $$log{'fhr'};
  }
  if ($$log{'fhw'}) {
    $$log{'fhw'}->close;
    undef $$log{'fhw'};
  }
  
  0;
}

sub register {
  my ($log, $op, $op_function) = @_;

  ${$$log{'op_table'}}{$op} = $op_function;
  0;
}

sub lock_reader {
  my ($log) = @_;
  flock($$log{'fhr'}, LOCK_SH);
}

sub unlock_reader {
  my ($log) = @_;
  flock($$log{'fhr'}, LOCK_UN);
}

sub lock_writer {
  my ($log) = @_;
  flock($$log{'fhw'}, LOCK_EX);
}

sub unlock_writer {
  my ($log) = @_;
  flock($$log{'fhw'}, LOCK_UN);
}

sub read_one {
  my ($log) = @_;
  my $line = '';

again:
  my ($rin, $win, $ein) = ('') x 3;
  vec($rin, fileno($$log{'fhr'}), 1) = 1;
  $ein = $rin | $win;
  print "select\n";
  my $n = select($rin, undef, undef, undef);
  print "select'\n";

  my ($c, $result);
  while (($result = sysread($$log{'fhr'}, $c, 1)) == 1) {
    last if $c eq "\n";
    $line .= $c;
  }
  
  $result or goto again;
  $line;
}

sub read {
  my ($log) = @_;
  my @lines = ( );
  
  while (1) {
    my ($c, $result, $line);
    while (($result = sysread($$log{'fhr'}, $c, 1)) == 1) {
      last if $c eq "\n";
      $line .= $c;
    }
    if ($result) {
      push(@lines, $line);
    } else {
      seek($$log{'fhr'}, -length($line) - 1, SEEK_CUR);
      last;
    }
  }
  
  @lines;
}

sub write {
  my ($log, @line) = @_;
  my $data = join(" ", @line) . "\n";
  syswrite($$log{'fhw'}, $data, length($data));
  0;
}

sub op_open {
  my ($log, $x) = @_;

  $$x{'status'} ne '' and die "dx::log::op_open error 0\n";
  ${$$log{'x_open'}}{$$x{'id'}} = $x;
  $$x{'status'} = 'open';
  0;
}

sub op_abort {
  my ($log, $x) = @_;

  $$x{'status'} ne 'open' and die "dx::log::op_abort error 0\n";
  undef ${$$log{'x_open'}}{$$x{'id'}};
  ${$$log{'x_closed'}}{$$x{'id'}} = $x;
  $$x{'status'} = 'aborted';
  0;
}

sub op_commit {
  my ($log, $x) = @_;

  $$x{'status'} ne 'open' and die "dx::log::op_commit error 0\n";
  undef ${$$log{'x_open'}}{$$x{'id'}};
  ${$$log{'x_closed'}}{$$x{'id'}} = $x;
  $$x{'status'} = 'committed';
  0;
}

sub op_lock_r {
  my ($log, $x, $op, $object_id) = @_;
  
  my $object = new dx::object 'id' => $object_id, 'log' => $log;
  $object->read;
  $$x{'status'} ne 'open' and die "dx::log::op_lock_r error 0\n";
  ${$$x{'asset_r'}}{$$object{'id'}} = 1;
  ${$$object{'owner_r'}}{$$x{'id'}} = 1;
  $object->write;

  0;
}

sub op_lock_rw {
  my ($log, $x, $op, $object_id) = @_;
  
  my $object = new dx::object 'id' => $object_id, 'log' => $log;
  $object->read;
  $$x{'status'} ne 'open' and die "dx::log::op_lock_r error 0\n";
  ${$$x{'asset_rw'}}{$$object{'id'}} = 1;
  ${$$object{'owner_rw'}}{$$x{'id'}} = 1;
  $object->write;

  0;
}

sub op_unlock {
  my ($log, $x, $op, $object_id) = @_;

  my $object = new dx::object 'id' => $object_id, 'log' => $log;
  $object->read;
  $$x{'status'} ne 'open' and die "dx::log::op_unlock error 0\n";
  delete ${$$x{'asset_r'}}{$$object{'id'}};
  delete ${$$x{'asset_rw'}}{$$object{'id'}};
  delete ${$$object{'owner_r'}}{$$x{'id'}};
  delete ${$$object{'owner_rw'}}{$$x{'id'}};
  $object->write;

  0;
}

sub sync_reader {
  my ($log) = @_;

  for my $line ($log->read) {
    my ($x_id, $op, @args) = split(/\s+/, $line);
    $x_id >= $$log{'x_next'} and $$log{'x_next'} = $x_id + 1;
    my $x = ${$$log{'x'}}{$x_id} ||=
     new dx::x 'log' => $log, 'id' => $x_id, 'status' => '';

    my $op_function = ${$$log{'op_table'}}{$op};
    !$op_function || $op_function->($log, $x, $op, @args) and
     die "dx::log::sync_reader error 0\n";
  }
  
  0;
}

sub sync_reader_one {
  my ($log) = @_;

  my $line = $log->read_one;
  my ($x_id, $op, @args) = split(/\s+/, $line);
  $x_id >= $$log{'x_next'} and $$log{'x_next'} = $x_id + 1;
  my $x = ${$$log{'x'}}{$x_id} ||=
   new dx::x 'log' => $log, 'id' => $x_id, 'status' => '';

  my $op_function = ${$$log{'op_table'}}{$op};
  !$op_function || $op_function->($log, $x, $op, @args) and
   die "dx::log::sync_reader_one error 0\n";
  
  0;
}


1;
