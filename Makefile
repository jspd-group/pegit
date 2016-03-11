CC=gcc
CFLAGS=-std=gnu11
INC=include/
SRC=src/
TEST=test/
ZLIB=z

all: test

debug: CFLAGS += -g

debug: all

test: test-mz test-strbuf test-file test-delta test-deltafile test-tree test-visitor test-init test-stage

test-mz: strbuf.o file.o mz.o test-mz.o
	$(CC) strbuf.o mz.o test-mz.o file.o -l$(ZLIB) -o test-mz.exe

test-strbuf: test-strbuf.o strbuf.o

test-file: test-file.o strbuf.o file.o test-file.o

test-delta: test-delta.o strbuf.o delta.o file.o delta-file.o

test-deltafile: test-deltafile.o delta-file.o file.o strbuf.o

test-init: strbuf.o timestamp.o project-config.o visitor.o project-init.o test-init.o

test-stage: strbuf.o test-stage.o visitor.o tree.o stage.o cache.o file.o mz.o
	$(CC) strbuf.o test-stage.o visitor.o tree.o stage.o cache.o file.o mz.o -lz -o test-stage

test-tree: strbuf.o tree.o test-tree.o

test-visitor: strbuf.o tree.o visitor.o test-visitor.o

cache.o: $(SRC)cache.c $(INC)cache.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)cache.c

stage.o: $(SRC)stage.c $(INC)stage.h
	$(CC) -I $(INC) -Isha1 $(CFLAGS) -c $(SRC)stage.c

strbuf.o: $(SRC)strbuf.c $(INC)strbuf.h $(INC)util.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)strbuf.c

mz.o: $(SRC)mz.c $(INC)mz.h
	$(CC) -I $(INC) $(CFLAGS) -lz -c $(SRC)mz.c

file.o: $(SRC)file.c $(INC)file.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)file.c

delta.o:$(SRC)delta.c $(INC)delta.h $(INC)strbuf-list.h delta-file.o
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)delta.c

delta-file.o:$(SRC)delta-file.c $(INC)delta-file.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)delta-file.c

project-config.o: $(SRC)project-config.c $(INC)project-config.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)project-config.c

project-init.o: $(SRC)project-init.c $(INC)project-init.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)project-init.c

timestamp.o: $(SRC)timestamp.c $(INC)timestamp.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)timestamp.c

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
	$(CC) -I $(INC) -c $(TEST)test-mz.c

test-deltafile.o: $(TEST)test-deltafile.c
	$(CC) -I $(INC) $(CFLAGS) -c $(TEST)test-deltafile.c

test-init.o: $(TEST)test-init.c
	$(CC) -I $(INC) $(CFLAGS) -c $(TEST)test-init.c

test-stage.o: $(TEST)test-stage.c stage.o
	$(CC) -I $(INC) $(CFLAGS) -c $(TEST)test-stage.c

test-tree.o: $(TEST)test-tree.c tree.o
	$(CC) -I $(INC) $(CFLAGS) -c $(TEST)test-tree.c

test-visitor.o: $(TEST)test-visitor.c tree.o visitor.o
	$(CC) -I $(INC) $(CFLAGS) -c $(TEST)test-visitor.c

clean:
	-@rm *.o *.exe
	-@rm -r .peg
