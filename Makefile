CC=gcc
CFLAGS=-std=gnu11
INC=include/
SRC=src/
TEST=test/
ZLIB=z

all: test

debug: CFLAGS += -g

debug: test

test: test-mz test-strbuf test-file test-delta test-deltafile test-tree test-visitor

test-mz: strbuf.o file.o mz.o test-mz.o
	$(CC) strbuf.o mz.o test-mz.o file.o -l$(ZLIB) -o test-mz.exe

test-strbuf: test-strbuf.o strbuf.o

test-file: test-file.o strbuf.o file.o test-file.o

test-delta: test-delta.o strbuf.o delta.o file.o delta-file.o

test-deltafile: test-deltafile.o delta-file.o file.o strbuf.o

test-tree: strbuf.o tree.o test-tree.o

test-visitor: strbuf.o tree.o visitor.o test-visitor.o

strbuf.o: $(SRC)strbuf.c $(INC)strbuf.h $(INC)util.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)strbuf.c

mz.o: $(SRC)mz.c $(INC)mz.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)mz.c

file.o: $(SRC)file.c $(INC)file.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)file.c

delta.o:$(SRC)delta.c $(INC)delta.h $(INC)strbuf-list.h delta-file.o
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)delta.c

delta-file.o:$(SRC)delta-file.c $(INC)delta-file.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)delta-file.c

tree.o:$(SRC)tree.c $(INC)tree.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)tree.c

visitor.o:$(SRC)visitor.c $(INC)visitor.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)visitor.c

test-strbuf.o: $(TEST)test-strbuf.c
	$(CC) -I $(INC) $(CFLAGS) -c $(TEST)test-strbuf.c

test-file.o: $(TEST)test-file.c
	$(CC) -I $(INC) $(CFLAGS) -c $(TEST)test-file.c

test-delta.o: $(TEST)test-delta.c
	$(CC) -I $(INC) $(CFLAGS) -c $(TEST)test-delta.c

test-mz.o: $(TEST)test-mz.c
	$(CC) -I $(INC) -I $(ZLIB) -c $(TEST)test-mz.c

test-deltafile.o: $(TEST)test-deltafile.c
	$(CC) -I $(INC) $(CFLAGS) -c $(TEST)test-deltafile.c

test-tree.o: $(TEST)test-tree.c tree.o
	$(CC) -I $(INC) $(CFLAGS) -c $(TEST)test-tree.c

test-visitor.o: $(TEST)test-visitor.c tree.o visitor.o
	$(CC) -I $(INC) $(CFLAGS) -c $(TEST)test-visitor.c

clean:
	@rm *.o *.exe