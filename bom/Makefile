# {HEADER}

include ../Makefile.inc

OBJ = \
  common.o db.o environ.o event.o object.o value.o \
  note.o acct.o denom.o offer.o tid.o

INC = $(INC_DIR)/bom.h

LIB = $(LIB_DIR)/libbom.a

LINC = $(INC_DIR)/dmath.h

all: $(INC) $(LIB)

$(LIB_DIR)/libbom.a: $(OBJ)
	$(AR) rs $(LIB_DIR)/libbom.a $(OBJ)
  
$(INC_DIR)/bom.h: bom.h
	install -m 0644 bom.h $(INC_DIR)

$(OBJ): $(INC) $(LINC)

clean:
	rm -f $(OBJ) $(LIB) $(INC) core
	 
install: all
	install -m 0644 $(LIB) $(INST_DIR)/lib
	install -m 0644 $(INC) $(INST_DIR)/include
