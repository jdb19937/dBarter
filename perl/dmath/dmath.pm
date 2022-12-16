package dmath;
require DynaLoader;
require AutoLoader;
require Exporter;

@ISA = qw(Exporter DynaLoader);

$VERSION = '0.01';
bootstrap dmath $VERSION;

_dmath_init();

use dmath::rsa;
use dmath::bz;
use dmath::bq;

@EXPORT = qw(
  rsa_crypt
  rsa_challenge
  rsa_keygen
);

1;

__END__
