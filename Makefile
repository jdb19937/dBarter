include Makefile.inc

all:
	make -C contrib
	make -C dmath
	make -C dthread
	make -C dx
	make -C bom
	make -C bif
	make -C barterd
	make -C perl
	make -C tools
	make -C html

clean:
	make -C contrib clean
	make -C dmath clean
	make -C dthread clean
	make -C dx clean
	make -C bom clean
	make -C bif clean
	make -C barterd clean
	make -C perl clean
	make -C tools clean
	make -C html clean
	rm -f dbarter-$(VERSION).tar.gz
	
install: all
	install -d $(INST_DIR)
	install -d $(INST_DIR)/bin
	install -d $(INST_DIR)/lib
	install -d $(INST_DIR)/include
	install -m 0644 VERSION $(INST_DIR)
	install -m 0644 LICENSE $(INST_DIR)
	install -m 0644 LICENSE.* $(INST_DIR)

	make -C contrib install
	make -C dmath install
	make -C dthread install
	make -C dx install
	make -C bom install
	make -C bif install
	make -C barterd install
	make -C perl install
	make -C tools install
	make -C html install

uninstall:
	rm -rf $(INST_DIR)
	
build: clean
	cd ..; tar -cf /tmp/dbarter-$(VERSION).tar dbarter-$(VERSION)
	gzip -9 -c /tmp/dbarter-$(VERSION).tar > dbarter-$(VERSION).tar.gz
	rm -f /tmp/dbarter-$(VERSION).tar
