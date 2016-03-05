#include "delta-file.h"
#include "strbuf.h"

static size_t count_lines(struct strbuf *buf)
{
    int count, ch, completely_empty = 1, nl = 0;
    int len = buf->len;
    char *b = buf->buf;

    count = 0;
    while (0 < len--) {
        ch = *(b++);
        if (ch == '\n') {
            count++;
            nl = 1;
            completely_empty = 0;
        } else {
            nl = 0;
            completely_empty = 0;
        }
    }
    if (completely_empty) return 0;
    if (!nl) count++;
    return count;
}

int split_in_place(struct strbuf *buf, ssize_t *arr, char delim)
{
    char *b = buf->buf;
    int count = 0, i = 0;
    arr[count++] = -1;
    for (i = 0; i < buf->len; i++) {
        if (b[i] == '\n') {
            arr[count++] = i;
        }
    }
    if (b[--i] != '\n') arr[count++] = i;

    return count;
}

int deltafile_init(struct deltafile *df, size_t size, char delim)
{
    df->arr = (ssize_t *)malloc(sizeof(ssize_t) * (size + 1));
    if (!df->arr) die("Out of memory.\n");

    df->size = size;
    df->delim = delim;
}

int deltafile_init_filespec(struct deltafile *df, struct filespec *fs, char d)
{
    strbuf_init(&df->file, fs->length);
    filespec_read_safe(fs, &df->file);
    return deltafile_init_strbuf(df, &df->file, d);
}

int deltafile_init_strbuf(struct deltafile *df, struct strbuf *buf, char delim)
{
    size_t lines = count_lines(buf);
    deltafile_init(df, lines + 1, delim);
    df->size = split_in_place(&df->file, df->arr, delim);
    return 0;
}

void deltafile_free(struct deltafile *df)
{
    strbuf_release(&df->file);
    free(df->arr);
    df->size = 0;
}