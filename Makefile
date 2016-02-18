CC=gcc
CFLAGS=-std=gnu11
INC=include/
SRC=src/
TEST=test/
ZLIB=miniz/

all: test

debug: CFLAGS += -g

debug: test

test: test_mz test_strbuf test-file test-delta

test_mz: strbuf.o mz.o file.o $(TEST)test-mz.c
	$(CC) -I $(INC) $(CFLAGS) -c $(TEST)test-mz.c
	$(CC) strbuf.o mz.o test-mz.o file.o -o test-mz

test_strbuf: $(TEST)test-strbuf.c strbuf.o
	$(CC) -I $(INC) $(CFLAGS) -c $(TEST)test-strbuf.c
	$(CC) strbuf.o test-strbuf.o -o test-strbuf

test-file: $(TEST)test-file.c strbuf.o
	$(CC) -I $(INC) $(CFLAGS) -c $(TEST)test-file.c
	$(CC) strbuf.o file.o test-file.o -o test-file

test-delta: test-delta.o strbuf.o delta.o file.o delta-file.o

strbuf.o: $(SRC)strbuf.c $(INC)strbuf.h $(INC)util.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)strbuf.c

file.o: $(SRC)file.c $(INC)file.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)file.c

mz.o: $(SRC)mz.c
	$(CC) -I $(INC) -I $(ZLIB) $(CFLAGS) -c $(SRC)mz.c

delta.o:$(SRC)delta.c $(INC)delta.h $(INC)strbuf-list.h delta-file.o
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)delta.c

delta-file.o:$(SRC)delta-file.c $(INC)delta-file.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)delta-file.c

test-delta.o: $(TEST)test-delta.c
	$(CC) -I $(INC) $(CFLAGS) -c $(TEST)test-delta.c


clean:
	rm *.o *.exe