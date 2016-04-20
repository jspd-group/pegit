#include "stage.h"
#include "cache.h"

static inline void
cache_index_entry_list_init(struct cache_index_entry_list *n)
{
    strbuf_init(&n->file_path, 0);
    n->next = NULL;
}

struct cache_index_entry_list* create_node(size_t start, size_t len)
{
    struct cache_index_entry_list *ne =
                    MALLOC(struct cache_index_entry_list, 1);
    cache_index_entry_list_init(ne);
    if (!ne)
        die("memory not available\n");
    ne->start = start;
    ne->len = len;
    return ne;
}

void clear_list_index(struct cache_index_entry_list *list)
{
    for (struct cache_index_entry_list *ptr = list, *temp = NULL;
        ptr != NULL; temp = ptr, ptr = ptr->next)
        if (temp)
            free(temp);
}

void cache_index_entry_list_insert(struct cache_index_entry_list **head,
                                   struct cache_index_entry_list **last,
                                   struct cache_index_entry_list *node)
{
    if (!*head) {
        *head = node;
        *last = node;
        return;
    }
    (*last)->next = node;
    *last = node;
}

void cache_index_init(struct cache_index *idx)
{
    idx->num = 0;
    author_init(&idx->auth);
    idx->entries = NULL;
    idx->idxfile = NULL;
    idx->flushed = true;
}

void open_index_file(struct cache_index *idx)
{
    idx->idxfile = fopen(CACHE_INDEX_FILE, "rwb");
    if (!idx->idxfile)
        die("can't read %s, %s\n", CACHE_INDEX_FILE, strerror(errno));
}

int read_index_node(struct cache_index *idx,
    struct cache_index_entry_list *node)
{
    struct strbuf buf = STRBUF_INIT;
    size_t len;

    fread(&node->start, sizeof(size_t), 1, idx->idxfile);
    fread(&node->len, sizeof(size_t), 1, idx->idxfile);
    fread(&len, sizeof(size_t), 1, idx->idxfile);
    strbuf_init(&buf, len);
    strbuf_fread(&buf, len, idx->idxfile);
    strbuf_addbuf(&node->file_path, &buf);
    strbuf_release(&buf);
    fread(&node->st, sizeof(struct stat), 1, idx->idxfile);
    fread(&node->status, sizeof(node->status), 1, idx->idxfile);
    return 0;
}

int read_cache_index_file(struct cache_index *idx)
{
    size_t start, len;
    struct cache_index_entry_list *n;
    int i = 0, ret;

    ret = fread(&idx->num, sizeof(size_t), 1, idx->idxfile);
    if (ret < 1)
        return -1;
    for (i = 0; i < idx->num; i++) {
        n = create_node(0, 0);
        n->next = NULL;
        if (read_index_node(idx, n))
            die("error while reading cache\n");
        cache_index_entry_list_insert(&idx->entries, &idx->last, n);
    }
    fclose(idx->idxfile);
    return (i == idx->num) ? 0 : -1;
}

void write_index_node(struct cache_index *idx,
                      struct cache_index_entry_list *n)
{
    if (!n)
        die("BUG: cache_index_entry_list node was NULL\n");
    fwrite(&n->start, sizeof(size_t), 1, idx->idxfile);
    fwrite(&n->len, sizeof(size_t), 1, idx->idxfile);
    fwrite(&n->file_path.len, sizeof(size_t), 1, idx->idxfile);
    fwrite(n->file_path.buf, sizeof(char), n->file_path.len, idx->idxfile);
    fwrite(&n->st, sizeof(struct stat), 1, idx->idxfile);
    fwrite(&n->status, sizeof(n->status), 1, idx->idxfile);
}

int write_index_file(struct cache_index *idx)
{
    int ret;
    struct cache_index_entry_list *n;

    idx->idxfile = fopen(CACHE_INDEX_FILE, "wb");
    if (!idx->idxfile) {
        die("%s, error occurred, %s\n", CACHE_INDEX_FILE, strerror(errno));
    }
    ret = fwrite(&idx->num, sizeof(size_t), 1, idx->idxfile);
    if (ret < 1)
        return -1;
    n = idx->entries;
    for (int i = 0; i < idx->num; i++) {
        write_index_node(idx, n);
        n = n->next;
    }
    idx->flushed = true;
    fclose(idx->idxfile);
    idx->idxfile = NULL;
    return 0;
}

void close_index_file(struct cache_index *idx)
{
    if (!idx->flushed)
        write_index_file(idx);
    fclose(idx->idxfile);
}

void make_index_entry(struct cache_index *idx, size_t pos, size_t len)
{
    cache_index_entry_list_insert(&idx->entries, &idx->last,
        create_node(pos, len));
    idx->num++;
    idx->flushed = false;
}

void make_index_entry_details(struct cache_index *idx,
                                struct cache_index_entry_list *node)
{
    cache_index_entry_list_insert(&idx->entries, &idx->last, node);
    idx->num++;
    idx->flushed = false;
}

void cache_init(struct cache *c)
{
    c->num_entries = 0;
    strbuf_init(&c->cache_buf, 0);
    c->cachefile = NULL;
    c->flushed = true;
}

void cache_release(struct cache *c)
{
    strbuf_release(&c->cache_buf);
    c->num_entries = 0;
    c->cachefile = NULL;
}

void open_cache_file(struct cache *c)
{
    c->cachefile = fopen(CACHE_PACK_FILE, "rwb");
    if (!c->cachefile)
        die("can't open %s\n", CACHE_PACK_FILE);
}

void read_cache_file(struct cache *c)
{
    size_t size = file_length(c->cachefile);
    strbuf_fread(&c->cache_buf, size, c->cachefile);
    fclose(c->cachefile);
}

void add_cache(struct cache *c, struct strbuf *buf)
{
    strbuf_addbuf(&c->cache_buf, buf);
    c->flushed = false;
    c->num_entries++;
}

void write_cache_file(struct cache *c)
{
    int ret;

    c->cachefile = fopen(CACHE_PACK_FILE, "wb");
    ret = fwrite(c->cache_buf.buf, sizeof(char),
                c->cache_buf.len, c->cachefile);
    if (ret < c->cache_buf.len)
        die("unable to write %s\n", CACHE_PACK_FILE);
    c->flushed = true;
    fclose(c->cachefile);
    c->cachefile = NULL;
}

void close_cache_file(struct cache *c)
{
    if (c->flushed) {
        fclose(c->cachefile);
        return;
    }
    write_cache_file(c);

}

void cache_object_init(struct cache_object *co)
{
    cache_index_init(&co->ci);
    cache_init(&co->cc);
    open_index_file(&co->ci);
    read_cache_index_file(&co->ci);
    open_cache_file(&co->cc);
    read_cache_file(&co->cc);
}

void cache_object_write(struct cache_object *co)
{
    write_cache_file(&co->cc);
    write_index_file(&co->ci);
}

void cache_object_add(struct cache_object *co, struct strbuf *buf)
{
    size_t size = co->cc.cache_buf.len;
    add_cache(&co->cc, buf);
    make_index_entry(&co->ci, size, buf->len);
}

void cache_object_addindex(struct cache_object *co, struct strbuf *buf,
                           struct cache_index_entry_list *node)
{
    size_t start = co->cc.cache_buf.len;
    add_cache(&co->cc, buf);
    node->start = start;
    make_index_entry_details(&co->ci, node);
}

void cache_object_add_file(struct cache_object *co, FILE *f)
{
    size_t len = file_length(f);
    struct strbuf buf = STRBUF_INIT;
    if (strbuf_fread(&buf, len, f) < len)
        die("unable to read file\n");
    cache_object_add(co, &buf);

    strbuf_release(&buf);
}

void cache_object_add_compressed_file(struct cache_object *co, FILE *f)
{
    size_t len = file_length(f);
    struct strbuf buf = STRBUF_INIT;
    struct strbuf dest = STRBUF_INIT;

    if (strbuf_fread(&buf, len, f) < len)
        die("unable to read file\n");
    __compress__(&buf, &dest, 9);
    cache_object_add(co, &dest);
    strbuf_release(&buf);
    strbuf_release(&dest);
}
