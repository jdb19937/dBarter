package bom::common;

use Exporter;

@ISA = qw(Exporter);

@EXPORT = qw(
  escape
  unescape
  try
  catch
  throw
  array
);

srand(time + $$);

sub unescape {
  my $s = shift;
  $s =~ tr/+/ /;
  $s =~ s/%([0-9a-fA-F]{2})/pack("c", hex($1))/ge;
  $s;
}

sub escape {
  my $s = shift;
  $s =~ s/([^a-zA-Z0-9_\-. ])/uc sprintf("%%%02x", ord($1))/eg;
  $s =~ tr/ /+/;
  $s;
}

sub try(&$) {
  my ($try, $catch) = @_;
  eval {
    &$try
  };
  if ($@) {
    $@ =~ /^THROW:(.*$)/;
    local $_ = unescape($1);
    &$catch;
  }
}

sub catch(&) {
  $_[0];
}

sub throw($) {
  my $object = shift;
  die "THROW:" . escape($object) . "\n";
}

sub array($) {
  my $ar = shift;
  if (ref($ar) eq 'ARRAY') {
    @$ar;
  } else {
    ($ar);
  }
}

sub bom_init {
  my $db_dir = shift || "/usr/local/barter/db";
  $bom::common::db_dir = $db_dir;
}

sub bom_exit {

}

1;
