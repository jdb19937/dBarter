package bom::value;

use dmath;
use bom::denom;
use overload '""' => \&to_string;

sub new($@) {
  my $class = shift;
  
  if (@_ == 2) {
    bless {
      'denom', => $_[0],
      'quant' => $_[1]
    }, $class;
  } else {
    die "invalid usage of bom::value::new\n";
  }
}

sub denom($;$) {
  my ($self, $value) = @_;

  if (defined($value)) {
    $self->{'denom'} = $value;
  } else {
    $self->{'denom'};
  }
}

sub quant($;$) {
  my ($self, $value) = @_;

  if (defined($value)) {
    $self->{'quant'} = $value;
  } else {
    $self->{'quant'};
  }
}

sub to_string {
  my $self = shift;
  $self->{'denom'}->id . '{' . $self->{'quant'}->to_string . '}';
}

sub new_from_string($) {
  my $class = shift;
  shift =~ /(^[^{]+)\{([^}]+)\}$/;
  bless {
    'denom' => bom::denom->new($1),
    'quant' => dmath::bq->new($2),
  }, $class;
}

1;
