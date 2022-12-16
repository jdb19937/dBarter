package dmath::bq;
require Exporter;

use overload
  '+' => \&add,
  '-' => \&subtract,
  '*' => \&mul,
  '/' => \&div,
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
  dmath::_bq_add($arg3, f($arg1), f($arg2));
  bless \$arg3, dmath::bq;
}

sub subtract {
  my ($arg1, $arg2, $switch) = @_;
  my $arg3;
  if ($switch) {
    dmath::_bq_sub($arg3, f($arg2), f($arg1));
  } else {
    dmath::_bq_sub($arg3, f($arg1), f($arg2));
  }
  bless \$arg3, dmath::bq;
}

sub mul {
  my ($arg1, $arg2) = @_;
  my $arg3;
  dmath::_bq_mul($arg3, f($arg1), f($arg2));
  bless \$arg3, dmath::bq;
}

sub div {
  my ($arg1, $arg2, $switch) = @_;
  my $arg3;
  if ($switch) {
    dmath::_bq_div($arg3, f($arg2), f($arg1));
  } else {
    dmath::_bq_div($arg3, f($arg1), f($arg2));
  }
  bless \$arg3, dmath::bq;
}

1;

__END__
