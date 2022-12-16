package dmath::rsa;
require Exporter;

@ISA = qw(Exporter);

@EXPORT = qw(
  rsa_keygen
  rsa_challenge
  rsa_crypt
);

sub rsa_keygen($) {
  my ($bits) = @_;
  my ($public, $secret, $modulus);
  dmath::_rsa_keygen($public, $secret, $modulus, $bits) and
   die "error in _rsa_keygen";
  (dmath::bz->new($public), dmath::bz->new($secret), dmath::bz->new($modulus));
}

sub rsa_crypt($$$) {
  my ($plain, $exponent, $modulus) = @_;
  my $cipher;
  dmath::_rsa_crypt($cipher, $$plain, $$exponent, $$modulus) and
   die "error in _rsa_crypt";
  new dmath::bz $cipher;
}

sub rsa_challenge($) {
  my ($modulus) = @_;
  my $challenge;
  dmath::_rsa_challenge($challenge, $$modulus) and
   die "error in _rsa_challenge";
  new dmath::bz $challenge;
}

1;

__END__
