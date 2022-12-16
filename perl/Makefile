include ../Makefile.inc

all:	bom/Makefile bif/Makefile dmath/Makefile
	make -C dmath
	make -C bom
	make -C bif

clean:  bom/Makefile bif/Makefile dmath/Makefile
	make -C dmath clean
	make -C bom clean
	make -C bif clean
	
install: all
	make -C dmath install
	make -C bom install
	make -C bif install

dmath/Makefile:
	cd dmath; perl Makefile.PL
bom/Makefile:
	cd bom; perl Makefile.PL
bif/Makefile:
	cd bif; perl Makefile.PL
