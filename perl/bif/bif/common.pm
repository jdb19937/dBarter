package bif::common;
use Exporter;

@ISA = qw(Exporter);

@EXPORT = qw(
  encode_string
  decode_string
  hash_to_string
  hash_from_string
);

my %enc_table = (
  "\n" => "\\n",
  "\a" => "\\a",
  "\f" => "\\f",
  "\b" => "\\b",
  "\t" => "\\t",
  "\r" => "\\r",
  "\"" => "\\\"",
  "\\" => "\\\\",
);

sub encode_string {
  my ($s) = @_;
  $s =~ s/([\a\n\f\b\t\r\"\\])/$enc_table{$1}/eg;
  $s =~ s/([^\ -\~])/'\x' . uc(sprintf("%02x", ord($1)))/eg;
  ($s ne $_[0] || $s =~ /\s/) ? "\"$s\"" : $s;
}

my %dec_table = reverse(%enc_table);

sub decode_string {
  my ($s) = @_;
  $s =~ /^"/ && $s =~ /"$/ or return $s;
  $s = substr($s, 1, -1);
  $s =~ s/(\\[anfbtr\"\\])/$dec_table{$1}/eg;
  $s =~ s/\\x([0-9a-fA-F]{2})/pack("c", hex($1))/eg;
  $s
}

sub hash_from_string($) {
  local $_ = shift;
  chomp;

  my $hash;
  for (split(/\n+/)) {
    my ($k, $v) = split(/ +/, $_, 2);
    $v =~ /^"/ and $v = decode_string($v);
    if (exists($$hash{$k})) {
      if (ref($$hash{$k})) {
        $$hash{$k} = [ @{$$hash{$k}}, $v ];
      } else {
        $$hash{$k} = [ $$hash{$k}, $v ];
      }
    } else {
      $$hash{$k} = $v;
    }
  }
  
  $hash;
}

sub hash_to_string($) {
  my $hash = shift;
  join "\n", map {
    my $k = $_;
    if (ref($$hash{$k})) {
      join "\n", map { $k . ' ' . encode_string($_) } @{$$hash{$k}}
    } else {
      $_ . ' ' . $$hash{$_}
    }
  } keys(%$hash);
}

1;
