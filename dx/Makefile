# {HEADER}

include ../Makefile.inc

OBJ = \
  common.o log.o x.o

INC = $(INC_DIR)/dx.h

LIB = $(LIB_DIR)/libdx.a

LINC = $(INC_DIR)/dthread.h

all: $(INC) $(LIB)

$(LIB_DIR)/libdx.a: $(OBJ)
	$(AR) rs $(LIB_DIR)/libdx.a $(OBJ)
  
$(INC_DIR)/dx.h: dx.h
	install -m 0644 dx.h $(INC_DIR)

$(OBJ): $(INC) $(LINC)

clean:
	rm -f $(OBJ) $(LIB) $(INC) core
	 
install: all
	install -m 0644 $(LIB) $(INST_DIR)/lib
	install -m 0644 $(INC) $(INST_DIR)/include
