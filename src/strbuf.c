#include "strbuf.h"
/*
 * The size parameter specifies the available space, i.e. includes
 * the trailing NUL byte; but Windows's vsnprintf uses the entire
 * buffer and avoids the trailing NUL, should the buffer be exactly
 * big enough for the result. Defining SNPRINTF_SIZE_CORR to 1 will
 * therefore remove 1 byte from the reported buffer size, so we
 * always have room for a trailing NUL byte.
 */
#ifndef SNPRINTF_SIZE_CORR
#if defined(WIN32) && (!defined(__GNUC__) || __GNUC__ < 4)
#define SNPRINTF_SIZE_CORR 1
#else
#define SNPRINTF_SIZE_CORR 0
#endif
#endif

#undef vsnprintf
int peg_vsnprintf(char *str, size_t maxsize, const char *format, va_list ap)
{
    va_list cp;
    char *s;
    int ret = -1;

    if (maxsize > 0) {
        va_copy(cp, ap);
        ret = vsnprintf(str, maxsize-SNPRINTF_SIZE_CORR, format, cp);
        va_end(cp);
        if (ret == maxsize-1)
            ret = -1;
        /* Windows does not NUL-terminate if result fills buffer */
        str[maxsize-1] = 0;
    }
    if (ret != -1)
        return ret;

    s = NULL;
    if (maxsize < 128)
        maxsize = 128;

    while (ret == -1) {
        maxsize *= 4;
        str = realloc(s, maxsize);
        if (! str)
            break;
        s = str;
        va_copy(cp, ap);
        ret = vsnprintf(str, maxsize-SNPRINTF_SIZE_CORR, format, cp);
        va_end(cp);
        if (ret == maxsize-1)
            ret = -1;
    }
    free(s);
    return ret;
}

int peg_snprintf(char *str, size_t maxsize, const char *format, ...)
{
    va_list ap;
    int ret;

    va_start(ap, format);
    ret = peg_vsnprintf(str, maxsize, format, ap);
    va_end(ap);

    return ret;
}


int starts_with(const char *str, const char *prefix)
{
	for (; ; str++, prefix++)
		if (!*prefix)
			return 1;
		else if (*str != *prefix)
			return 0;
}

/*
 * Used as the default ->buf value, so that people can always assume
 * buf is non NULL and ->buf is NUL terminated even for a freshly
 * initialized strbuf.
 */
char strbuf_slopbuf[1];

void strbuf_init(struct strbuf *sb, size_t hint)
{
	sb->alloc = sb->len = 0;
	sb->buf = strbuf_slopbuf;
	if (hint)
		strbuf_grow(sb, hint);
}

void strbuf_release(struct strbuf *sb)
{
	if (sb->alloc) {
		free(sb->buf);
		strbuf_init(sb, 0);
	}
}

char *strbuf_detach(struct strbuf *sb, size_t *sz)
{
	char *res;
	strbuf_grow(sb, 0);
	res = sb->buf;
	if (sz)
		*sz = sb->len;
	strbuf_init(sb, 0);
	return res;
}

void strbuf_attach(struct strbuf *sb, void *buf, size_t len, size_t alloc)
{
	strbuf_release(sb);
	sb->buf   = buf;
	sb->len   = len;
	sb->alloc = alloc;
	strbuf_grow(sb, 0);
	sb->buf[sb->len] = '\0';
}

void strbuf_grow(struct strbuf *sb, size_t extra)
{
	int new_buf = !sb->alloc;
	if (new_buf)
		sb->buf = NULL;
	ALLOC_GROW(sb->buf, sb->len + extra + 1, sb->alloc);
	if (new_buf)
		sb->buf[0] = '\0';
}

void strbuf_trim(struct strbuf *sb)
{
	strbuf_rtrim(sb);
	strbuf_ltrim(sb);
}
void strbuf_rtrim(struct strbuf *sb)
{
	while (sb->len > 0 && isspace((unsigned char)sb->buf[sb->len - 1]))
		sb->len--;
	sb->buf[sb->len] = '\0';
}

void strbuf_ltrim(struct strbuf *sb)
{
	char *b = sb->buf;
	while (sb->len > 0 && isspace(*b)) {
		b++;
		sb->len--;
	}
	memmove(sb->buf, b, sb->len);
	sb->buf[sb->len] = '\0';
}

void strbuf_tolower(struct strbuf *sb)
{
	char *p = sb->buf, *end = sb->buf + sb->len;
	for (; p < end; p++)
		*p = tolower(*p);
}

int strbuf_cmp(const struct strbuf *a, const struct strbuf *b)
{
	int len = a->len < b->len ? a->len: b->len;
	int cmp = memcmp(a->buf, b->buf, len);
	if (cmp)
		return cmp;
	return a->len < b->len ? -1: a->len != b->len;
}

void strbuf_splice(struct strbuf *sb, size_t pos, size_t len,
				   const void *data, size_t dlen)
{
	if (pos > sb->len)
		die("`pos' is too far after the end of the buffer");
	if (pos + len > sb->len)
		die("`pos + len' is too far after the end of the buffer");

	if (dlen >= len)
		strbuf_grow(sb, dlen - len);
	memmove(sb->buf + pos + dlen,
			sb->buf + pos + len,
			sb->len - pos - len);
	memcpy(sb->buf + pos, data, dlen);
	strbuf_setlen(sb, sb->len + dlen - len);
}

void strbuf_insert(struct strbuf *sb, size_t pos, const void *data, size_t len)
{
	strbuf_splice(sb, pos, 0, data, len);
}

void strbuf_remove(struct strbuf *sb, size_t pos, size_t len)
{
	strbuf_splice(sb, pos, len, NULL, 0);
}

void strbuf_add(struct strbuf *sb, const void *data, size_t len)
{
	strbuf_grow(sb, len);
	memcpy(sb->buf + sb->len, data, len);
	strbuf_setlen(sb, sb->len + len);
}

void strbuf_adddup(struct strbuf *sb, size_t pos, size_t len)
{
	strbuf_grow(sb, len);
	memcpy(sb->buf + sb->len, sb->buf + pos, len);
	strbuf_setlen(sb, sb->len + len);
}

void strbuf_addchars(struct strbuf *sb, int c, size_t n)
{
	strbuf_grow(sb, n);
	memset(sb->buf + sb->len, c, n);
	strbuf_setlen(sb, sb->len + n);
}

static void add_lines(struct strbuf *out,
			const char *prefix1,
			const char *prefix2,
			const char *buf, size_t size)
{
	while (size) {
		const char *prefix;
		const char *next = memchr(buf, '\n', size);
		next = next ? (next + 1) : (buf + size);

		prefix = ((prefix2 && (buf[0] == '\n' || buf[0] == '\t'))
			  ? prefix2 : prefix1);
		strbuf_addstr(out, prefix);
		strbuf_add(out, buf, next - buf);
		size -= next - buf;
		buf = next;
	}
	strbuf_complete_line(out);
}

size_t strbuf_fread(struct strbuf *sb, size_t size, FILE *f)
{
	size_t res;
	size_t oldalloc = sb->alloc;

	strbuf_grow(sb, size);
	res = fread(sb->buf + sb->len, 1, size, f);
	if (res > 0)
		strbuf_setlen(sb, sb->len + res);
	else if (oldalloc == 0)
		strbuf_release(sb);
	return res;
}

ssize_t strbuf_read(struct strbuf *sb, int fd, size_t hint)
{
	size_t oldlen = sb->len;
	size_t oldalloc = sb->alloc;

	strbuf_grow(sb, hint ? hint : 8192);
	for (;;) {
		ssize_t want = sb->alloc - sb->len - 1;
		ssize_t got = read(fd, sb->buf + sb->len, want);

		if (got < 0) {
			if (oldalloc == 0)
				strbuf_release(sb);
			else
				strbuf_setlen(sb, oldlen);
			return -1;
		}
		sb->len += got;
		if (got < want)
			break;
		strbuf_grow(sb, 8192);
	}

	sb->buf[sb->len] = '\0';
	return sb->len - oldlen;
}

ssize_t strbuf_read_once(struct strbuf *sb, int fd, size_t hint)
{
	ssize_t cnt;

	strbuf_grow(sb, hint ? hint : 8192);
	cnt = read(fd, sb->buf + sb->len, sb->alloc - sb->len - 1);
	if (cnt > 0)
		strbuf_setlen(sb, sb->len + cnt);
	return cnt;
}

#define STRBUF_MAXLINK (2*PATH_MAX)

int strbuf_getcwd(struct strbuf *sb)
{
	size_t oldalloc = sb->alloc;
	size_t guessed_len = 128;

	for (;; guessed_len *= 2) {
		strbuf_grow(sb, guessed_len);
		if (getcwd(sb->buf, sb->alloc)) {
			strbuf_setlen(sb, strlen(sb->buf));
			return 0;
		}
		if (errno != ERANGE)
			break;
	}
	if (oldalloc == 0)
		strbuf_release(sb);
	else
		strbuf_reset(sb);
	return -1;
}

#ifdef HAS_GETDELIM

int strbuf_getwholeline(struct strbuf *sb, FILE *fp, int term)
{
	ssize_t r;

	if (feof(fp))
		return EOF;

	strbuf_reset(sb);

	/* Translate slopbuf to NULL, as we cannot call realloc on it */
	if (!sb->alloc)
		sb->buf = NULL;
	r = getdelim(&sb->buf, &sb->alloc, term, fp);

	if (r > 0) {
		sb->len = r;
		return 0;
	}
	assert(r == -1);

	if (errno == ENOMEM)
		die("Out of memory, getdelim failed");

	/* Restore slopbuf that we moved out of the way before */
	if (!sb->buf)
		strbuf_init(sb, 0);
	return EOF;
}
#else
int strbuf_getwholeline(struct strbuf *sb, FILE *fp, int term)
{
	int ch;

	if (feof(fp))
		return EOF;

	strbuf_reset(sb);
	//flockfile(fp);
	while ((ch = fgetc(fp)) != EOF) {
		if (!strbuf_avail(sb))
			strbuf_grow(sb, 1);
		sb->buf[sb->len++] = ch;
		if (ch == term)
			break;
	}
	//funlockfile(fp);
	if (ch == EOF && sb->len == 0)
		return EOF;

	sb->buf[sb->len] = '\0';
	return 0;
}
#endif

int strbuf_getline(struct strbuf *sb, FILE *fp, int term)
{
	if (strbuf_getwholeline(sb, fp, term))
		return EOF;
	if (sb->buf[sb->len-1] == term)
		strbuf_setlen(sb, sb->len-1);
	return 0;
}

// int strbuf_getwholeline_fd(struct strbuf *sb, int fd, int term)
// {
// 	strbuf_reset(sb);

// 	while (1) {
// 		char ch;
// 		ssize_t len = read(fd, &ch, 1);
// 		if (len <= 0)
// 			return EOF;
// 		strbuf_addch(sb, ch);
// 		if (ch == term)
// 			break;
// 	}
// 	return 0;
// }

ssize_t strbuf_read_file(struct strbuf *sb, const char *path, size_t hint)
{
	int fd;
	ssize_t len;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return -1;
	len = strbuf_read(sb, fd, hint);
	close(fd);
	if (len < 0)
		return -1;

	return len;
}

void strbuf_add_lines(struct strbuf *out, const char *prefix,
		      const char *buf, size_t size)
{
	add_lines(out, prefix, NULL, buf, size);
}

void extract_filename(struct strbuf *name, const char *path)
{
    size_t size = strlen(path);
    int len = (int)size;

    while (--len >= 0 && path[len] != '/')
        /* do nothing */;

    if (len >= 0)
        size = size - len - 1;
    else {
        len = 0;
    }
    strbuf_reset(name);
    strbuf_add(name, path + len + 1, size);
}

size_t line_size(char *str)
{
	int count = 0;

	while (*str++ != '\n')
		count++;
	return count + 1;
}

int strbuf_split(struct strbuf *buf, struct strbuf *out, char delim)
{
	int i = 0;
	int hint = 80;
	int size = 0;
	char *temp = buf->buf;

	while (*temp) {
		strbuf_init(out + i, 80);
		size = line_size(temp);

		strbuf_add(out + i, temp, size);
		temp = temp + size;
		i++;
	}

	return i;
}

void strbuf_addf(struct strbuf *sb, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    strbuf_vaddf(sb, fmt, ap);
    va_end(ap);
}

void strbuf_vaddf(struct strbuf *sb, const char *fmt, va_list ap)
{
    int len;
    va_list cp;

    if (!strbuf_avail(sb))
        strbuf_grow(sb, 64);
    va_copy(cp, ap);
    len = peg_vsnprintf(sb->buf + sb->len, sb->alloc - sb->len, fmt, cp);
    va_end(cp);
    if (len < 0)
        die("BUG: your vsnprintf is broken (returned %d)", len);
    if (len > strbuf_avail(sb)) {
        strbuf_grow(sb, len);
        len = peg_vsnprintf(sb->buf + sb->len, sb->alloc - sb->len, fmt, ap);
        if (len > strbuf_avail(sb))
            die("BUG: your vsnprintf is broken (insatiable)");
    }
    strbuf_setlen(sb, sb->len + len);
}

int inplace_compare(const char *a, const char *b, size_t sa, size_t sb)
{
  int len = sa < sb ? sa : sb;
  int cmp = memcmp(a, b, len);
  if (cmp)
    return cmp;
  return sa < sb ? -1 : sa != sb;
}

void strbuf_replace_chars(struct strbuf *sb, char replace, char subs)
{
    for (int i = 0; i < sb->len; sb++) {
        if (sb->buf[i] == replace)
            sb->buf[i] = subs;
    }
}
