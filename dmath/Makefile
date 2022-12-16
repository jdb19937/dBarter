include ../Makefile.inc

OBJ = rsa.o bz.o bq.o environ.o

LIB = $(LIB_DIR)/libdmath.a

INC = $(INC_DIR)/dmath.h

all: $(INC) $(LIB)
  
$(LIB_DIR)/libdmath.a: $(OBJ)
	$(AR) rs $(LIB_DIR)/libdmath.a $(OBJ)
  
$(INC_DIR)/dmath.h:
	install -m 0644 dmath.h $(INC_DIR)

$(OBJ): $(INC)

clean:
	rm -f $(OBJ) $(LIB) $(INC) core
	 
install: all
	install -m 0644 $(LIB) $(INST_DIR)/lib
	install -m 0644 $(INC) $(INST_DIR)/include
