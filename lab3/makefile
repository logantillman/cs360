CC = gcc 
INCLUDES = -I/home/jplank/cs360/include
CFLAGS = $(INCLUDES)

EXECUTABLES = bin/huff_dec

all: $(EXECUTABLES)

bin/huff_dec: src/huff_dec.c
	$(CC) $(CFLAGS) -o bin/huff_dec src/huff_dec.c

clean:
	rm core $(EXECUTABLES) *.o
