package bom::portfolio;

use dmath;
use bom::value;
use overload '""' => \&to_string;

sub new($@) {
  my ($class, %hash) = @_;
  bless \%hash, $class;
}

sub new_from_values($@) {
  my ($class, @values) = @_;

  my $self = bless { }, $class;
  for (@values) {
    $$self{$_->denom->id} = $_->quant;
  }
  
  $self;
}

sub new_from_string($$) {
  my ($class, $string) = @_;
  
  $class->new_from_values(
    map { new_from_string bom::value $_ } split(/\s+/, $string)
  );
}

sub to_string {
  my $self = shift;
  join " ", map { $_->to_string } $self->values;
}

sub to_string_list {
  my $self = shift;
  map { $_->to_string } $self->values;
}

sub denoms {
  my $self = shift;
  map { new bom::denom $_ } keys %$self;
}

sub quant($$) {
  my ($self, $denom) = @_;
  $self->{$denom->id} or new dmath::bq 0;
}

sub value($$) {
  my ($self, $denom) = @_;
  new bom::value $denom, $self->quant($denom);
}

sub values($) {
  my $self = shift;
  map { new bom::value bom::denom->new($_), $$self{$_} } keys(%$self);
}

1;
