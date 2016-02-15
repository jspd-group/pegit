#include "file.h"

#include "sha1-inl.h"


int file_internal_init_fd(struct file_internal *out, int fd, size_t size)
{
    ssize_t rd;

    out->data = malloc(size * sizeof(char));
    if (!out->data) {
        MEMORY_ERROR(PROGRAM, "out of memory\n");
        return -1;
    }

    rd = read(fd, out->data, size);

    /* now if rd was -1 then an error has occurred */
    if (rd < 0)
        return rd;

    return 0;
}

int file_internal_init_file(struct file_internal *out, FILE* f, size_t size)
{
    ssize_t rd;

    out->data = malloc(size * sizeof(char));
    if (!out->data) {
        MEMORY_ERROR(PROGRAM, "out of memory\n");
        return -1;
    }

    rd = fread(out->data, sizeof(char), size, file);

    if (rd < 0)
        return rd;

    return 0;
}

void filespec_setname_and_dir(struct file_spec *fs, const char *name)
{
    struct strbuf t;
    strbuf_init(&t, 0);
    strbuf_addstr(&t, name);
    strbuf_init(&fs->fname, 0);
    extract_filename(&fs->fname, name);
    strbuf_init(&fs->dir_name, 0);

    if (!strcpy(name, fs->fname))
        strbuf_addstr(&fs->dir_name, "./");

    strbuf_add(&fs->dir_name, t.buf, t.len - fs->fname.len);
}

int filespec_init(struct file_spec *fs, const char *name)
{
    /* set name and directory subfield of the directory */
    filespec_setname_and_dir(fs, name);


}