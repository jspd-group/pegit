#include "file.h"

#include "sha1-inl.h"
#include <sys/types.h>
#include <sys/stat.h>

int filecache_init(struct filecache *fi)
{
    fi->data = (struct strbuf *)malloc(sizeof(struct strbuf));
    if (!fi->data) error("out of memory\n");
    strbuf_init(fi->data, 0);
    return 0;
}

// int filecache_init_fd(struct filecache *out, int fd, size_t size)
// {
//     ssize_t rd;

//     rd = strbuf_read(out->data, fd, size);

//     /* now if rd was -1 then an error has occurred */
//     if (rd < 0)
//         return rd;

//     return 0;
// }

int filecache_init_file(struct filecache *out, FILE *f, size_t size)
{
    strbuf_fread(out->data, size, f);
    return 0;
}

void filecache_free(struct filecache *fc)
{
    strbuf_release(fc->data);
    free(fc->data);
}

void filespec_setname_and_dir(struct filespec *fs, const char *name)
{
    struct strbuf t;
    strbuf_init(&t, 0);
    strbuf_addstr(&t, name);
    strbuf_init(&fs->fname, 0);
    extract_filename(&fs->fname, name);
    strbuf_init(&fs->dir_name, 0);

    if (!strcmp(name, fs->fname.buf)) strbuf_addstr(&fs->dir_name, "./");

    strbuf_add(&fs->dir_name, t.buf, t.len - fs->fname.len);

    strbuf_release(&t);
}

size_t file_length(FILE *file)
{
    size_t pos = ftell(file);
    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    fseek(file, pos, SEEK_SET);
    return length;
}

void clean_on_error(struct filespec *fs)
{
    strbuf_release(&fs->fname);
    strbuf_release(&fs->dir_name);
}

void filespec_getfullpath(struct filespec *fs, struct strbuf *buf)
{
    strbuf_addbuf(buf, &fs->dir_name);
    strbuf_addbuf(buf, &fs->fname);
}

int filespec_stat(struct filespec *fs, const char *name)
{
    if (stat(name, &fs->st) == -1) {
        fprintf(stderr, "fatal: %s: can't do stat\n", name);
        return -1;
    }
    return 0;
}

int filespec_init(struct filespec *fs, const char *name, const char *mode)
{
    /* set name and directory subfield of the directory */
    filespec_setname_and_dir(fs, name);
    fs->file = fopen(name, mode);
    if (!fs->file) {
        fprintf(stderr, "%s: file doesn't exists.\n", name);
        clean_on_error(fs); /* remove the memory allocated */
        return -1;
    }

    if (filespec_stat(fs, name) == -1) {
        clean_on_error(fs);
        return -1;
    }

    fs->last_modified = fs->st.st_mtime;
    fs->length = file_length(fs->file);

    /* postpone the file caching until call to filespec read takes place */
    filecache_init(&fs->cache);
    fs->cached = 0;
    return 0;
}

int filespec_cachefile(struct filespec *fs)
{
    int size = file_length(fs->file);
    struct strbuf err;

    if (filecache_init_file(&fs->cache, fs->file, size) < 0) {
        strbuf_init(&err, 0);
        filespec_getfullpath(fs, &err);
        error("%s: error occurred while caching file.\n", err.buf);
        strbuf_release(&err);
        return -1;
    }
    fs->cached = 1;
    return 0;
}

int filespec_read_unsafe(struct filespec *fs, struct strbuf *buf, int fast)
{
    int err;
    if (!fs->cached) {
        err = filespec_cachefile(fs);
        if (err) return -1;
        fs->cached = 1;
    }

    /**
     * using fast call is risky. Because if, by any chance, you used
     * strbuf_release on the input buffer, then the cache will be in
     * undefined state, may lead to many errors. So, always use 0.
     */
    if (fast) {
        buf->buf = fs->cache.data->buf;
        buf->len = fs->cache.data->len;
        buf->alloc = fs->cache.data->alloc;
        return 0;
    }

    strbuf_init(buf, fs->cache.data->len);
    strbuf_addbuf(buf, fs->cache.data);
    return 0;
}

void filespec_reset(struct filespec *fs)
{
    if (fseek(fs->file, 0, SEEK_SET) < 0)
        error("Unable to seek <%s> to the beginning\n", fs->fname.buf);
}

int filespec_read_safe(struct filespec *fs, struct strbuf *buf)
{
    strbuf_init(buf, fs->length);
    strbuf_fread(buf, fs->length, fs->file);
    filespec_reset(fs);
    return 0;
}

void filespec_remove_cache(struct filespec *fs)
{
    strbuf_release(fs->cache.data);
    fs->cached = 0;
}

int filespec_sha1(struct filespec *fs, char sha1[20])
{
    struct strbuf buf;
    strbuf_init(&buf,
                sizeof(time_t) + fs->fname.len + fs->dir_name.len + fs->length);

    if (!fs->cached) {
        if (filespec_cachefile(fs) < 0) return -1;
    }

    strbuf_addbuf(&buf, &fs->fname);
    strbuf_addbuf(&buf, &fs->dir_name);
    strbuf_add(&buf, (char *)&fs->last_modified, sizeof(time_t));
    strbuf_addbuf(&buf, fs->cache.data);
    strtosha1(&buf, sha1);

    strbuf_release(&buf);
    return 0;
}

void filespec_free(struct filespec *fs)
{
    strbuf_release(&fs->fname);
    strbuf_release(&fs->dir_name);
    filecache_free(&fs->cache);
    fclose(fs->file);
}