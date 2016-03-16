#ifndef STAGE_H_
#define STAGE_H_

#include "util.h"
#include "strbuf.h"
#include "commit.h"
#include "cache.h"

struct stage_stats {
    size_t files_modified;
    size_t new_files;
    size_t total;
} stats;

struct stage_options {
    int all;
    int ignore;
    int all_dot;
    struct strbuf *ignarr;
    struct strbuf *add;
};

struct file_list {
    struct strbuf path;
    struct strbuf file;
    short sha1[HASH_SIZE];
    bool old;
    struct file_list *next;
};

extern int stage_main(int argc, char *argv[]);

#endif
