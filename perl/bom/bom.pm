package bom;
require DynaLoader;
require AutoLoader;
require Exporter;

@ISA = qw(Exporter DynaLoader);

$VERSION = '0.01';
bootstrap bom $VERSION;

_bom_init();

use dmath;
use bom::common;
use bom::object;
use bom::denom;
use bom::value;
use bom::portfolio;
use bom::txn;
use bom::acct;
use bom::note;
use bom::event;
use bom::offer;

$bom::db_initialized = 0;
$bom::db_directory = "";

sub bom_db_init {
  return -1 if $bom::db_initialized;
  $bom::db_directory = $_[0] || '/usr/local/barter/db';
  _bom_db_init($bom::db_directory);
  $bom::db_initialized = 1;
  0;
}

sub bom_db_exit {
  return -1 unless $bom::db_initialized;
  _bom_db_exit();
  $bom::db_directory = "";
  $bom::db_initialized = 0;
  0;
}

sub bom_db_get($$;$) {
  return -1 unless $bom::db_initialized;
  my ($file, $key, $txn) = @_;
  $txn ||= $bom::txn::default;

  my $value;
  if (my $result = _bom_db_get($file, $$txn, $key, $value)) {
    throw $result;
  }
  
  $value;
}

sub bom_db_put($$$;$) {
  return -1 unless $bom::db_initialized;
  my ($file, $key, $value, $txn) = @_;
  $txn ||= $bom::txn::default;

  if (my $result = _bom_db_put($file, $$txn, $key, $value)) {
    throw $result;
  }
  
  $value;
}

sub bom_db_del($$$;$) {
  return -1 unless $bom::db_initialized;
  my ($file, $key, $txn) = @_;
  $txn ||= $bom::txn::default;

  if (my $result = _bom_db_del($file, $$txn, $key)) {
    throw $result;
  }

  1;
}

sub try(&$) {
  my ($code, $catch) = @_;
  bom::common::try(\&{$_[0]}, $_[1]);
}

sub try_txn(&$) {
  bom::txn::try_txn(\&{$_[0]}, $_[1]);
}

sub throw($) {
  bom::common::throw($_[0]);
}

sub catch(&) {
  bom::common::catch(\&{$_[0]});
}

@EXPORT = qw(
  bom_db_init
  bom_db_exit
  bom_db_get
  bom_db_put
  bom_db_del
  try
  try_txn
  throw
  catch
);

END {
  bom::bom_db_exit;
}

1;

__END__
