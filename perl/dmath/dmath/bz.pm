package dmath::bz;
require Exporter;

use overload
  '+' => \&add,
  '-' => \&subtract,
  '*' => \&mul,
  '/' => \&div,
  '%' => \&mod,
  '**' => \&pow,
  '""' => \&to_string;

sub new($;$) {
  my ($class, $string) = @_;
  $string ||= "0";
  bless \$string, $class;
}

sub to_string {
  my $self = shift;
  $$self;
}

sub f {
  my $self = shift;
  ref($self) ? $$self : $self;
}

sub add {
  my ($arg1, $arg2) = @_;
  my $arg3;
  dmath::_bz_add($arg3, f($arg1), f($arg2));
  bless \$arg3, dmath::bz;
}

sub subtract {
  my ($arg1, $arg2, $switch) = @_;
  my $arg3;
  if ($switch) {
    dmath::_bz_sub($arg3, f($arg2), f($arg1));
  } else {
    dmath::_bz_sub($arg3, f($arg1), f($arg2));
  }
  bless \$arg3, dmath::bz;
}

sub mul {
  my ($arg1, $arg2) = @_;
  my $arg3;
  dmath::_bz_mul($arg3, f($arg1), f($arg2));
  bless \$arg3, dmath::bz;
}

sub div {
  my ($arg1, $arg2, $switch) = @_;
  my $arg3;
  if ($switch) {
    dmath::_bz_div($arg3, f($arg2), f($arg1));
  } else {
    dmath::_bz_div($arg3, f($arg1), f($arg2));
  }
  bless \$arg3, dmath::bz;
}

sub mod {
  my ($arg1, $arg2) = @_;
  my $arg3;
  dmath::_bz_mod($arg3, f($arg1), f($arg2));
  bless \$arg3, dmath::bz;
}

sub pow {
  my ($arg1, $arg2, $switch) = @_;
  my $arg3;

  if ($switch) {
    dmath::_bz_pow($arg3, f($arg2), f($arg1));
  } else {
    dmath::_bz_pow($arg3, f($arg1), f($arg2));
  }
  bless \$arg3, dmath::bz;
}

1;

__END__
