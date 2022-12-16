package bom::txn;
use POSIX qw(:errno_h);
use bom::common;

use Exporter;

@ISA = qw(Exporter);

@EXPORT = qw(
  DB_INCOMPLETE
  DB_KEYEMPTY
  DB_KEYEXIST
  DB_LOCK_DEADLOCK
  DB_LOCK_NOTGRANTED
  DB_LOCK_NOTHELD
  DB_NOTFOUND
  DB_RUNRECOVERY
  DB_DELETED
  DB_NEEDSPLIT
  DB_SWAPBYTES
  DB_TXN_CKP
  try_txn
  commit
  abort
);

$bom::txn::null_id = 0;
$bom::txn::null = bless \$bom::txn::null_id, bom::txn;
$bom::txn::default = $bom::txn::null;

sub DB_INCOMPLETE () {( -1);}
sub DB_KEYEMPTY () {( -2);}
sub DB_KEYEXIST () {( -3);}
sub DB_LOCK_DEADLOCK () {( -4);}
sub DB_LOCK_NOTGRANTED () {( -5);}
sub DB_LOCK_NOTHELD () {( -6);}
sub DB_NOTFOUND () {( -7);}
sub DB_RUNRECOVERY () {( -8);}
sub DB_DELETED () {( -9);}
sub DB_NEEDSPLIT () {(-10);}
sub DB_SWAPBYTES () {(-11);}
sub DB_TXN_CKP () {(-12);}

sub new($) {
  my ($class) = @_;

  my $id;
  return undef if bom::_txn_new($id);
  bless \$id, $class;
}

sub abort {
  my $self = shift || $bom::txn::default;
  $$self or return -1;
  my $result = bom::_txn_abort($$self);
  $$self = 0;
  $result;
}

sub commit {
  my $self = shift || $bom::txn::default;
  $$self or return -1;
  my $result = bom::_txn_commit($$self);
  $$self = 0;
  $result;
}

sub DESTROY {
  my $self = shift;
  $$self and $self->abort;
}

sub try_txn(&$) {
  my ($code, $catch) = @_;
  my $status = 1;
  my $redo = 0;

  do {
   $bom::txn::default = new bom::txn;

    try {
      &$code;
    } catch {
      $status = $_;
      abort;
      $status == EAGAIN and $redo = 1;
    };
  } while ($redo);

  if ($$bom::txn::default) {
    commit;
    0;
  } else {
    local $_ = $status;
    &$catch if ($catch);
    $status;
  }
}

1;
