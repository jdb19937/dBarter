package dx::object;

sub new {
  my ($pkg, %param) = @_;
  
  my $object = bless {
    'id' => undef,
    'log' => undef,
    'owner_r' => { },
    'owner_rw' => { },
    %param,
  }, $pkg;
}

sub read {
  my ($object) = @_;
  
  my $log = $$object{'log'};
  my $index = $$log{'object'};
  %$object = ('id' => $$object{'id'}, %{$index{$$object{'id'}}});
  $$object{'owner_r'} = { %{$$object{'owner_r'}} };
  $$object{'owner_rw'} = { %{$$object{'owner_rw'}} };
  $$object{'log'} = $log;
  $object;
}

sub write {
  my ($object) = @_;

  my $log = $$object{'log'};
  my $index = $$log{'object'};

  $index{$$object{'id'}} = bless { %$object }, ref($object);
  $index{$$object{'id'}}->{'owner_r'} =
   { %{$index{$$object{'id'}}->{'owner_r'}} };
  $index{$$object{'id'}}->{'owner_rw'} =
   { %{$index{$$object{'id'}}->{'owner_rw'}} };
  undef ${$$index{$$object{'id'}}}{'log'};

  $object;
}

1;
