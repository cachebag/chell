# Makefile
CC = gcc
CFLAGS = -Wall -Wextra -O2
SRC = $(wildcard src/*.c)
BIN = main

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(BIN)

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN)
