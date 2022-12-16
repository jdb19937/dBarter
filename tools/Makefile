include ../Makefile.inc

BIN = $(BIN_DIR)/rsa_keygen $(BIN_DIR)/rsa_crypt \
      $(BIN_DIR)/acct_create $(BIN_DIR)/acct_read $(BIN_DIR)/acct_set \
      $(BIN_DIR)/event_listen $(BIN_DIR)/event_dump

all: $(BIN)
	true

$(BIN_DIR)/rsa_keygen: rsa_keygen
	install rsa_keygen $(BIN_DIR)
$(BIN_DIR)/rsa_crypt: rsa_crypt
	install rsa_crypt $(BIN_DIR)
$(BIN_DIR)/acct_create: acct_create
	install acct_create $(BIN_DIR)
$(BIN_DIR)/acct_read: acct_read
	install acct_read $(BIN_DIR)
$(BIN_DIR)/acct_set: acct_set
	install acct_set $(BIN_DIR)
$(BIN_DIR)/event_listen: event_listen
	install event_listen $(BIN_DIR)
$(BIN_DIR)/event_dump: event_dump
	install event_dump $(BIN_DIR)

clean:
	rm -f $(BIN)
	
install: all
	install -m 0775 $(BIN_DIR)/rsa_keygen $(INST_DIR)/bin
	install -m 0775 $(BIN_DIR)/rsa_crypt $(INST_DIR)/bin
	install -m 0775 $(BIN_DIR)/acct_create $(INST_DIR)/bin
	install -m 0775 $(BIN_DIR)/acct_read $(INST_DIR)/bin
	install -m 0775 $(BIN_DIR)/acct_set $(INST_DIR)/bin
	install -m 0775 $(BIN_DIR)/event_listen $(INST_DIR)/bin
	install -m 0775 $(BIN_DIR)/event_dump $(INST_DIR)/bin
