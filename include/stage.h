#ifndef STAGE_H_
#define STAGE_H_

#include "util.h"
#include "strbuf.h"
#include "commit.h"
#include "cache.h"

struct file_list {
    struct strbuf path;
    struct strbuf file;
    short sha1[HASH_SIZE];
    bool old;
    struct file_list *next;
};

extern int stage_main(int argc, char *argv[]);

extern int status_main(int argc, char* argv[]);

extern void show_cache_table();
#endif
