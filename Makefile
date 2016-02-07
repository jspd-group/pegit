CC=gcc
CFLAGS=-std=gnu11

all: test

debug: CFLAGS += -g

debug: test

test: strbuf.o test_mz test_strbuf

test_mz: mz.o test-mz.c
	$(CC) $(CFLAGS) -c test-mz.c
	$(CC) strbuf.o mz.o test-mz.o -o test-mz

test_strbuf: test-strbuf.c
	$(CC) $(CFLAGS) -c test-strbuf.c
	$(CC) strbuf.o test-strbuf.o -o test-strbuf

strbuf.o: strbuf.c strbuf.h util.h
	$(CC) $(CFLAGS) -c strbuf.c

mz.o:mz.c
	$(CC) $(CFLAGS) -c mz.c

clean:
	rm *.o *.exe