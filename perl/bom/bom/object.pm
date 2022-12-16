package bom::object; 
use UNIVERSAL qw(isa can);
use overload '""' => \&to_string;

sub new($;$) {
  my ($class, $id) = @_;
  
  if ($id) {
    bless { 'id' => $id }, $class;
  } else {
    bless { }, $class;
  }
}

sub id($;$) {
  my ($self, $value) = @_;

  if (defined($value)) {
    $self->{'id'} = $value;
  } else {
    $self->{'id'};
  }
}

sub to_string {
  my $self = shift;
  $self->id;
}

1;

