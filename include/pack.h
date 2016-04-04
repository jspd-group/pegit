#ifndef PACK_H_
#define PACK_H_

#include "strbuf.h"
#include "util.h"
#include "file.h"
#include "index.h"

struct pack_file_cache {
    char *pack_file_path; /* path to the pack file */
    struct strbuf cache; /* pack file is cached for now,
                            which can consume more memory */
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
    struct strbuf *buf, struct index *idx)
{
    strbuf_init(buf, idx->pack_len);
    if (idx->pack_start + idx->pack_len > cache->cache.len)
        return -1;
    strbuf_add(buf, cache->cache.buf + idx->pack_start, idx->pack_len);
    return 0;
}

static inline void invalidate_cache(struct pack_file_cache *cache)
{
    strbuf_release(&cache->cache);
}

#endif
