include ../Makefile.inc

OBJ = barterd.o
INC = barterd.h

LFLAGS = -lpthread -lm

BIN =  $(BIN_DIR)/barterd $(BIN_DIR)/barterd.start $(BIN_DIR)/barterd.stop

LOBJ = $(LIB_DIR)/libbif.a $(LIB_DIR)/libbom.a $(LIB_DIR)/libdthread.a \
       $(LIB_DIR)/libdmath.a $(LIB_DIR)/libdb.a $(LIB_DIR)/libgmp.a

LINC = $(INC_DIR)/bif.h $(INC_DIR)/bom.h $(INC_DIR)/dthread.h \
       $(INC_DIR)/dmath.h
       
all: $(BIN)

$(BIN_DIR)/barterd: $(OBJ) $(LOBJ)
	$(CXX) -o $(BIN_DIR)/barterd $(CFLAGS) $(OBJ) $(LOBJ) $(LFLAGS)
  
$(BIN_DIR)/barterd.start: barterd.start
	install barterd.start $(BIN_DIR)

$(BIN_DIR)/barterd.stop: barterd.stop
	install barterd.stop $(BIN_DIR)

$(OBJ): barterd.h $(LINC)

clean:
	rm -f $(OBJ) $(BIN) core
	
install: $(BIN)
	install -d $(INST_DIR)/db
	install -m 0755 $(BIN_DIR)/barterd $(INST_DIR)/bin
	install -m 0755 $(BIN_DIR)/barterd.start $(INST_DIR)/bin
	install -m 0755 $(BIN_DIR)/barterd.stop $(INST_DIR)/bin
