include ../Makefile.inc

OBJ = thread.o mutex.o mutex_rw.o condition.o

LIB = $(LIB_DIR)/libdthread.a

INC = $(INC_DIR)/dthread.h

all: $(INC) $(LIB)

$(LIB_DIR)/libdthread.a: $(OBJ)
	$(AR) rs $(LIB_DIR)/libdthread.a $(OBJ)
  
$(INC_DIR)/dthread.h:
	install -m 0644 dthread.h $(INC_DIR)

$(OBJ): $(INC)

clean:
	rm -f $(OBJ) $(LIB) $(INC) core
	 
install: all
	install -m 0644 $(LIB) $(INST_DIR)/lib
	install -m 0644 $(INC) $(INST_DIR)/include
