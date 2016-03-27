CC=gcc
CFLAGS=-std=gnu11
INC=include/
SRC=src/
TEST=test/
ZLIB=z

all: test peg

debug: CFLAGS += -g

debug: all

release: CFLAGS += -O2

release: all


test:  test-commit test-mz test-strbuf test-file test-delta test-deltafile test-tree test-visitor test-init test-stage test-status

peg: strbuf.o main.o cache.o file.o mz.o index.o visitor.o delta.o delta-file.o timestamp.o commit.o path.o project-init.o project-config.o stage.o checkout.o tree.o
	$(CC) strbuf.o main.o cache.o file.o mz.o index.o visitor.o delta.o delta-file.o timestamp.o commit.o path.o project-init.o project-config.o stage.o checkout.o -lz tree.o -o peg


test-mz: strbuf.o file.o mz.o test-mz.o
	$(CC) strbuf.o mz.o test-mz.o file.o -l$(ZLIB) -o test-mz.exe

test-strbuf: test-strbuf.o strbuf.o

test-file: test-file.o strbuf.o file.o test-file.o

test-delta: test-delta.o strbuf.o delta.o file.o delta-file.o commit.o stage.o mz.o visitor.o timestamp.o index.o cache.o tree.o path.o
	$(CC) strbuf.o test-delta.o visitor.o tree.o stage.o cache.o file.o mz.o commit.o delta.o delta-file.o timestamp.o index.o path.o -lz -o test-delta

test-deltafile: test-deltafile.o delta-file.o file.o strbuf.o

test-init: strbuf.o timestamp.o project-config.o visitor.o project-init.o test-init.o

test-stage: strbuf.o test-stage.o visitor.o tree.o stage.o cache.o file.o mz.o commit.o delta.o 	delta-file.o timestamp.o index.o path.o
	$(CC) strbuf.o test-stage.o visitor.o tree.o stage.o cache.o file.o mz.o commit.o delta.o delta-file.o timestamp.o index.o path.o -lz -o test-stage

test-commit: strbuf.o test-commit.o cache.o file.o mz.o index.o visitor.o delta.o  delta-file.o timestamp.o commit.o path.o
	$(CC) strbuf.o test-commit.o cache.o file.o mz.o index.o visitor.o delta.o	delta-file.o timestamp.o commit.o path.o -lz -o test-commit

test-checkout: strbuf.o checkout.o cache.o file.o mz.o index.o visitor.o delta.o  delta-file.o timestamp.o commit.o path.o
	$(CC) strbuf.o checkout.o cache.o file.o mz.o index.o visitor.o delta.o	delta-file.o timestamp.o commit.o path.o -lz -o test-checkout


test-status: strbuf.o status.o cache.o file.o mz.o index.o visitor.o delta.o  delta-file.o timestamp.o commit.o
	$(CC) strbuf.o status.o cache.o file.o mz.o index.o visitor.o delta.o	path.o delta-file.o timestamp.o commit.o -lz -o test-status

test-tree: strbuf.o tree.o test-tree.o

test-visitor: strbuf.o tree.o visitor.o test-visitor.o

cache.o: $(SRC)cache.c $(INC)cache.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)cache.c

commit.o: $(SRC)commit.c $(INC)commit.h $(INC)global.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)commit.c

checkout.o: $(SRC)checkout.c $(INC)global.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)checkout.c

stage.o: $(SRC)stage.c $(INC)stage.h
	$(CC) -I $(INC) -Isha1 $(CFLAGS) -c $(SRC)stage.c

strbuf.o: $(SRC)strbuf.c $(INC)strbuf.h $(INC)util.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)strbuf.c

mz.o: $(SRC)mz.c $(INC)mz.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)mz.c

main.o: $(SRC)main.c
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)main.c


file.o: $(SRC)file.c $(INC)file.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)file.c

delta.o:$(SRC)delta.c $(INC)delta.h $(INC)strbuf-list.h delta-file.o
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)delta.c

delta-file.o:$(SRC)delta-file.c $(INC)delta-file.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)delta-file.c

index.o:$(SRC)index.c $(INC)index.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)index.c

path.o:$(SRC)path.c $(INC)path.h
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)path.c

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

test-commit.o: $(TEST)test-commit.c
	$(CC) -I $(INC) $(CFLAGS) -c $(TEST)test-commit.c

test-delta.o: $(TEST)test-delta.c
	$(CC) -I $(INC) $(CFLAGS) -c $(TEST)test-delta.c

test-mz.o: $(TEST)test-mz.c
	$(CC) -I $(INC) $(CFLAGS) -c $(TEST)test-mz.c

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

status.o: $(SRC)status.c
	$(CC) -I $(INC) $(CFLAGS) -c $(SRC)status.c	

clean:
	-@rm *.o test-*

