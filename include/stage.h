#ifndef STAGE_H_
#define STAGE_H_

#include "cache.h"
#include "commit.h"
#include "strbuf.h"
#include "util.h"

struct file_list {
    struct strbuf path;
    struct strbuf file;
    short sha1[HASH_SIZE];
    bool old;
    int status;
    struct stat st;
    struct file_list *next;
};

extern int stage_main(int argc, char *argv[]);

extern int status_main(int argc, char *argv[]);

extern void show_cache_table();
#endif
