package bif;
$VERSION = '0.1';

use bom;
use bif::common;
use bif::client;

use Exporter;
@ISA = qw(Exporter);

@EXPORT = qw(
  encode_string
  decode_string
  hash_from_string
  hash_to_string
);

1;
