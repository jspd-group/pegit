#ifndef CACHE_H_
#define CACHE_H_

#include "strbuf.h"
#include "mz.h"
#include "tree.h"
#include "visitor.h"
#include "util.h"
#include "author.h"
#include "timestamp.h"


struct cache_index_entry_list {
    size_t start;
    size_t len;
    struct strbuf file_path;
    struct stat st;
    int status;
    short sha1[HASH_SIZE];
    struct cache_index_entry_list *next;
};

struct cache_index {
    size_t num;
    struct author auth;
    FILE *idxfile;
    bool flushed;
    struct cache_index_entry_list *entries;
    struct cache_index_entry_list *last;
};

struct cache {
    int num_entries;
    struct strbuf cache_buf;
    FILE *cachefile;
    bool flushed;
};

struct cache_object {
    struct cache cc;
    struct cache_index ci;
};

extern void cache_object_add_compressed_file(struct cache_object *co, FILE *f);
extern void cache_object_add_file(struct cache_object *co, FILE *f);
extern void cache_object_add(struct cache_object *co, struct strbuf *buf);
extern void cache_object_write(struct cache_object *co);
extern void cache_object_init(struct cache_object *co);
extern void cache_object_addindex(struct cache_object *co, struct strbuf *buf,
                           struct cache_index_entry_list *node);

static inline void cache_object_clean(struct cache_object *co)
{
    size_t size = 0;
    FILE *f = fopen(CACHE_INDEX_FILE, "wb");
    if (!f) die("unable to clear the staged data\n");
    fwrite(&size, sizeof(size_t), 1, f);
    fclose(f);
    f = fopen(CACHE_PACK_FILE, "wb");
    if (!f) die("unable to clear the staged data\n");
    fclose(f);
}

static bool find_file_from_cache(const char *name, struct cache_object *co)
{
    struct cache_index_entry_list *node = co->ci.entries;
    while (node) {
        if (!strcmp(node->file_path.buf, name))
            return true;
        node = node->next;
    }
    return false;
}

#endif
