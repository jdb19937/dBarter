use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    'CC' => 'g++',
    'NAME'	=> 'dmath',
    'VERSION_FROM' => 'dmath.pm', # finds $VERSION
    'LIBS'	=> [ '-lgmp' ],
    'DEFINE'	=> '',
    'INC'	=> '-I../../include',
    'MYEXTLIB' => '../../lib/libdmath$(LIB_EXT)',
    'OPTIMIZE' => '-O0',
);

