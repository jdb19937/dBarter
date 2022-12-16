include ../Makefile.inc

OBJ = \
  environ.o site.o common.o \
  server.o socket.o session.o \
  shell.o client.o message.o \
  market.o trader.o
  
LIB = $(LIB_DIR)/libbif.a

INC = $(INC_DIR)/bif.h
  
LINC = $(INC_DIR)/bom.h $(INC_DIR)/dthread.h $(INC_DIR)/dmath.h

all: $(INC) $(LIB)

$(LIB_DIR)/libbif.a: $(OBJ)
	$(AR) rs $(LIB_DIR)/libbif.a $(OBJ)
  
$(INC_DIR)/bif.h:
	install -m 0644 bif.h $(INC_DIR)

$(OBJ): $(INC) $(LINC)

clean:
	rm -f $(OBJ) $(LIB) $(INC) core
	 
install: all
	install -m 0644 $(LIB) $(INST_DIR)/lib
	install -m 0644 $(INC) $(INST_DIR)/include
