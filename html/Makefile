include ../Makefile.inc

all:
	true
clean:
	true
install:
	install -d $(INST_DIR)/html
	cp -rf . $(INST_DIR)/html
	rm -f $(INST_DIR)/html/Makefile
	# chown -R www $(INST_DIR)/html
	true
uninstall:
	rm -rf $(INST_DIR)/html
