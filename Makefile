CC=gcc
CFLAGS=-std=gnu11
INC=include/
SRC=src/
TEST=test/
ZLIB=miniz/

all: test

debug: CFLAGS += -g

debug: test

test: strbuf.o test_mz test_strbuf test-file

test_mz: mz.o file.o $(TEST)test-mz.c
	$(CC) -I $(INC) $(CFLAGS) -c $(TEST)test-mz.c
	$(CC) strbuf.o mz.o test-mz.o file.o -o test-mz

test_strbuf: $(TEST)test-strbuf.c
	$(CC) -I $(INC) $(CFLAGS) -c $(TEST)test-strbuf.c
	$(CC) strbuf.o test-strbuf.o -o test-strbuf

test-file: $(TEST)test-file.c
	$(CC) -I $(INC) $(CFLAGS) -c $(TEST)test-file.c

strbuf.o: $(SRC)strbuf.c $(INC)strbuf.h $(INC)util.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)strbuf.c

file.o: $(SRC)file.c $(INC)file.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)file.c

mz.o: $(SRC)mz.c
	$(CC) -I $(INC) -I $(ZLIB) $(CFLAGS) -c $(SRC)mz.c

clean:
	rm *.o *.exe