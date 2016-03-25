#ifndef PACK_H_
#define PACK_H_

#include "strbuf.h"
#include "util.h"
#include "file.h"

/* code for making packfile and index file and maintaining them. */
/**
 * pack file aka peg.pack will contain the files in compressed form and whose
 * The layout of the packfile is shown below...
 *    __________________
 *    |    header      |
 *    |________________|
 *    |     file1      |
 *    |                |
 *    |                |
 *    |________________|
 *    |     file2      |
 *    |                |
 *    |                |
 *    |________________|
 *    |                |
 *    |                |
 *    ------------------
 *
 * Only the contents of the file are stored in the pack file. Other details
 * are stored in different file called index file. Header contains:
 *    8 bytes: number of files
 *    120 bytes: reserved
 */

struct pack_file_cache {
    char *pack_file_path;
    struct strbuf cache;
};

#define PACK_FILE_CACHE_INIT { PACK_FILE, STRBUF_INIT }

static inline int cache_pack_file(struct pack_file_cache *cache)
{
    FILE *f = fopen(PACK_FILE, "rb");
    struct strbuf temp = STRBUF_INIT;

    if (!f) die("unable to open pack file, %s\n", strerror(errno));
    size_t size = file_length(f);
    strbuf_fread(&temp, size, f);
    if (size != 0) {
        decompress(&temp, &cache->cache);
    }
    fclose(f);
    strbuf_release(&temp);
    return 0;
}

static inline int flush_pack_cache(struct pack_file_cache *cache)
{
    int ret = 0;
    struct strbuf temp = STRBUF_INIT;
    FILE *cache_file = fopen(cache->pack_file_path, "wb");

    compress_default(&cache->cache, &temp);
    if (!cache_file)
        die("unable to pack file, %s\n", strerror(errno));
    ret = fwrite(temp.buf, sizeof(char), temp.len, cache_file);
    fclose(cache_file);
    strbuf_release(&temp);
    return (ret == cache->cache.len);
}

static inline int get_file_content(struct pack_file_cache *cache,
    struct strbuf *buf, size_t start, size_t len)
{
    strbuf_init(buf, len);
    if (start + len > cache->cache.len)
        return -1;
    strbuf_add(buf, cache->cache.buf + start, len);
    return 0;
}

static inline void invalidate_cache(struct pack_file_cache *cache)
{
    strbuf_release(&cache->cache);
}

#endif
