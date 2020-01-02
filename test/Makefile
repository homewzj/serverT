CC = gcc
CFLAGS := -g -Wall -O0
SRC := $(shell pwd)
LOG := $(SRC)/../log.o
LOG_TEST := log_test.o
INCLUDE := -I ../ 
BIN := log_test 
OBJ += $(LOG_TEST) $(LOG)
CFLAGS += $(INCLUDE)
debug:$(OBJ)
	$(CC) -o $(BIN) $(OBJ)
clean:
	rm -rf $(BIN) $(OBJ)
