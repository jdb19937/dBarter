use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    'CC' => 'g++',
    'NAME'	=> 'bom',
    'VERSION_FROM' => 'bom.pm', # finds $VERSION
    'LIBS'	=> [ '-L../../lib -ldb -ldmath -lgmp' ],
    'DEFINE'	=> '',
    'INC'	=> '-I../../include',
    'MYEXTLIB' => '../../lib/libbom$(LIB_EXT)',
    'OPTIMIZE' => '-O0',
);

