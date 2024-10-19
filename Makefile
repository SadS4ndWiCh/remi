CC := cc
CFLAGS := -Wall -Wextra

SRCS := $(wildcard ./src/*.c)

build: $(SRCS)
	$(CC) $(SRCS) $(CFLAGS) -o remi

client: client/main.c
	$(CC) ./client/main.c $(CFLAGS) -o ./client/client