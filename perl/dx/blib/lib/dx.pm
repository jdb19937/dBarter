package dx;

use dx::common;
use dx::log;
use dx::x;
use dx::datum;
use dx::lock;

use Exporter;
@ISA = qw(Exporter);

@EXPORT = qw(
  encode_string
  decode_string
  hash_from_string
  hash_to_string
);

1;
