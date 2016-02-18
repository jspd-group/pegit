#include "delta-file.h"
#include "strbuf.h"

size_t count_lines(struct strbuf *buf)
{
	char *str = buf->buf;
	size_t count;

	while (*str)
		if (*str++ == '\n')
			count++;

	if (*(--str) != '\n')
		count++;

	return count;
}

int deltafile_init(struct deltafile *df, size_t size, char delim)
{
	df->arr = (struct strbuf*)malloc(sizeof(struct deltafile) * size);
	if (!df->arr)
		die("Out of memory.\n");

	df->size = size;
	df->delim = delim;
}

int deltafile_init_filespec(struct deltafile *df, struct filespec *fs, char d)
{
	struct strbuf buf;
	strbuf_init(&buf, fs->length);
	filespec_read_safe(fs, &buf);
	return deltafile_init_strbuf(df, &buf, d);
}

int deltafile_init_strbuf(struct deltafile *df, struct strbuf *buf, char delim)
{
	size_t lines = count_lines(buf);
	deltafile_init(df, lines, delim);
	df->size = strbuf_split(buf, df->arr, delim);
	return 0;
}

void deltafile_free(struct deltafile *df)
{
	for (int i = 0; i < df->size; i++)
		strbuf_release(df->arr + i);
	free(df->arr);
	df->size = 0;
}