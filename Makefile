# C compiler we're using
CC= gcc

# C flags REQUIRED
CFLAGS= -std=gnu11
INC= include/
SRC= src/
TEST= test/
ZLIB= -lz
UNAME := $(shell uname)
EXECUTABLE_NAME=peg
INSTALL_DIR=''

# for zlib
ifdef NO_ZLIB
	CFLAGS+= -DNO_ZLIB
	ZLIB=
endif

ifdef NO_COLOR
	CFLAGS+= -DNO_COLOR
endif

# install utils
ifeq ($(OS), Windows_NT)
    UNAME= Win32
    INSTALL_PREFIX= C:/Program\ Files/Pegit/
    EXECUTABLE_NAME= peg.exe
    INSTALL_DIR= C:/Program\ Files/Pegit
endif

ifeq ($(UNAME), Linux)
    INSTALL_PREFIX= "/usr/local/bin"
endif

INSTALL_CMD= install
MKDIR_CMD= mkdir

# targets
all: peg

install: peg
	@-$(MKDIR_CMD) $(INSTALL_DIR)
	$(INSTALL_CMD) $(EXECUTABLE_NAME) $(INSTALL_PREFIX)

debug: CFLAGS += -g

debug: all

release: CFLAGS += -O2

release: all

peg: strbuf.o main.o cache.o file.o mz.o index.o visitor.o delta.o delta-file.o timestamp.o commit.o path.o project-init.o project-config.o stage.o checkout.o tree.o status.o
	@$(CC) *.o $(ZLIB) -o $(EXECUTABLE_NAME)
	@echo "Building for $(UNAME)"
	@echo $@

cache.o: $(SRC)cache.c $(INC)cache.h
	@$(CC) -I $(INC) $(CFLAGS) -c $(SRC)cache.c
	@echo $@

commit.o: $(SRC)commit.c $(INC)commit.h $(INC)global.h
	@$(CC) -I $(INC) $(CFLAGS) -c $(SRC)commit.c
	@echo $@

checkout.o: $(SRC)checkout.c $(INC)global.h
	@$(CC) -I $(INC) $(CFLAGS) -c $(SRC)checkout.c
	@echo $@

stage.o: $(SRC)stage.c $(INC)stage.h
	@$(CC) -I $(INC) -Isha1 $(CFLAGS) -c $(SRC)stage.c
	@echo $@

strbuf.o: $(SRC)strbuf.c $(INC)strbuf.h $(INC)util.h
	@$(CC) -I $(INC) $(CFLAGS) -c $(SRC)strbuf.c
	@echo $@

mz.o: $(SRC)mz.c $(INC)mz.h
	@$(CC) -I $(INC) $(CFLAGS) -c $(SRC)mz.c
	@echo $@

main.o: $(SRC)main.c
	@$(CC) -I $(INC) $(CFLAGS) -c $(SRC)main.c
	@echo $@

file.o: $(SRC)file.c $(INC)file.h
	@-$(CC) -I $(INC) $(CFLAGS) -c $(SRC)file.c
	@echo $@

delta.o:$(SRC)delta.c $(INC)delta.h $(INC)strbuf-list.h delta-file.o
	@-$(CC) -I $(INC) $(CFLAGS) -c $(SRC)delta.c
	@echo $@

delta-file.o:$(SRC)delta-file.c $(INC)delta-file.h
	@$(CC) -I $(INC) $(CFLAGS) -c $(SRC)delta-file.c
	@echo $@

index.o:$(SRC)index.c $(INC)index.h
	@$(CC) -I $(INC) $(CFLAGS) -c $(SRC)index.c
	@echo $@

path.o:$(SRC)path.c $(INC)path.h
	@$(CC) -I $(INC) $(CFLAGS) -c $(SRC)path.c
	@echo $@

project-config.o: $(SRC)project-config.c $(INC)global.h
	@$(CC) -I $(INC) $(CFLAGS) -c $(SRC)project-config.c
	@echo $@

project-init.o: $(SRC)project-init.c $(INC)project-init.h
	@$(CC) -I $(INC) $(CFLAGS) -c $(SRC)project-init.c
	@echo $@

timestamp.o: $(SRC)timestamp.c $(INC)timestamp.h
	@$(CC) -I $(INC) $(CFLAGS) -c $(SRC)timestamp.c
	@echo $@

tree.o:$(SRC)tree.c $(INC)tree.h
	@$(CC) -I $(INC) $(CFLAGS) -c $(SRC)tree.c
	@echo $@

visitor.o:$(SRC)visitor.c $(INC)visitor.h
	@$(CC) -I $(INC) $(CFLAGS) -c $(SRC)visitor.c
	@echo $@

status.o: $(SRC)status.c
	@$(CC) -I $(INC) $(CFLAGS) -c $(SRC)status.c
	@echo $@

help:
	@echo "Following targets are available."
	@echo " debug:      compiles \`peg' with debugging information so that you can use gdb"
	@echo " release:    compiles the program to small executable"
	@echo " all:        compiles the program without any debugging information"
	@echo " help:       displays this information"
	@echo " clean:      remove the executables and object files"
	@echo " install:    installs \`peg' to this computer"
	@echo ""
	@echo " Before compiling the program, please refer to the ReadMe.md"

clean:
	-@rm *.o peg

