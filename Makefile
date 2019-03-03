# Reference: http://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/
CC = gcc
CFLAGS = -g -lm -Wall -pedantic
DEPS = cache.h set.h line.h stats.h
OBJ = Cachelab.o cache.o set.o line.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

Cachelab: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
