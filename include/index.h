#ifndef INDEX_H_
#define INDEX_H_

/* Header file for maintaining index file */

#include "tree.h"
#include "strbuf.h"

struct index {
    char filename[256];
    struct stat st;
    uint8_t flags;
    size_t ver;
    size_t pack_start;
    size_t pack_len;
};

enum index_flag {
    IF_MODIFIED,
    IF_NEW,
    IF_DELETED
};

struct index_file_cache {
    char *index_file_path;    /* path of the index file */
    struct strbuf cache; /* whole file cache */
};

#define INDEX_CACHE_INIT { FILE_INDEX_FILE, STRBUF_INIT }

extern int cache_index_file(struct index_file_cache *);
extern struct index *make_new_index(const char *, size_t start,
    size_t len, uint8_t f);
extern struct index *make_index_from_cache(struct index_file_cache *cache,
    size_t offset);

static inline int flush_index(struct index_file_cache *cache)
{
    int ret = 0;
    FILE *idx_file = fopen(cache->index_file_path, "wb");

    if (!idx_file)
        die("unable to open index file\n");
    ret = fwrite(cache->cache.buf, sizeof(char), cache->cache.len, idx_file);
    fclose(idx_file);
    return (ret == cache->cache.len);
}

#endif
