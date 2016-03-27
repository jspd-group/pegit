#include "index.h"

void index_init(struct index *idx)
{
    memset(idx, sizeof(idx), 0);
}

struct index *make_new_index(const char *fn, size_t s,
                                size_t l, uint8_t f)
{
    struct index *idx = MALLOC(struct index, 1);
    memcpy(idx->filename, fn, strlen((char*)fn) + 1);
    idx->flags = f;
    idx->pack_start = s;
    idx->pack_len = l;
    return idx;
}

int cache_index_file(struct index_file_cache *cache)
{
    FILE *f = fopen(FILE_INDEX_FILE, "rb");
    if (!f) die("unable to open %s, %s\n", FILE_INDEX_FILE, strerror(errno));
    size_t size = file_length(f);
    strbuf_fread(&cache->cache, size, f);
    fclose(f);
    return 0;
}

struct index *make_index_from_cache(struct index_file_cache *cache,
    size_t start)
{
    struct index *idx = make_new_index("", 0, 0, 0);
    memcpy(idx, cache->cache.buf + start, sizeof(struct index));
    return idx;
}
