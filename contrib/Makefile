include ../Makefile.inc

INC = $(INC_DIR)/db.h $(INC_DIR)/gmp.h

LIB = $(LIB_DIR)/libdb.a $(LIB_DIR)/libgmp.a

all:	$(INC) $(LIB)
  
$(INC_DIR)/db.h:
	install -m 0644 db.h $(INC_DIR)
$(INC_DIR)/gmp.h:
	install -m 0644 gmp.h $(INC_DIR)
$(LIB_DIR)/libdb.a:
	install -m 0644 libdb.a $(LIB_DIR)
$(LIB_DIR)/libgmp.a:
	install -m 0644 libgmp.a $(LIB_DIR)

clean:
	rm -f $(INC) $(LIB) core
	
install: all
	install -m 0644 $(LIB) $(INST_DIR)/lib
	install -m 0644 $(INC) $(INST_DIR)/include
