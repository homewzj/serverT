#makefile

CC := gcc
CFLAGS += -g -Wall -O0
MSGQUEUE := msgQueue.o
MAIN := main.o
THREAD := thread.o
THREADPOOL := threadpool.o
LOG := log.o 
LD := -lpthread
BIN := server  
SERVER := server.o 
CONFIG := config.o
OBJ += $(MSGQUEUE) $(THREAD) $(THREADPOOL) $(MAIN) $(SERVER) $(LOG) $(CONFIG)
debug: $(OBJ)
	$(CC) -o $(BIN) $(OBJ) $(LD)

clean:
	rm -rf $(BIN)
	rm -rf $(OBJ)

