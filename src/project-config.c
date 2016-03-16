#include "project-config.h"
#include "strbuf.h"

void project_details_init(struct project_details *pd)
{
    strbuf_init(&pd->project_name, 256);
    strbuf_init(&pd->authors, 1024);
    strbuf_init(&pd->project_desc, 1024);
    strbuf_init(&pd->start_date, 256);
}

void project_details_free(struct project_details *pd)
{
    strbuf_release(&pd->project_name);
    strbuf_release(&pd->authors);
    strbuf_release(&pd->start_date);
    strbuf_release(&pd->project_desc);
}

void set_project_name(struct project_details *pd, const char *name)
{
    strbuf_setlen(&pd->project_name, 0);
    strbuf_addstr(&pd->project_name, name);
}

static inline bool
match_name(const char *str1, const char *str2, int pos, int len)
{
    return !strncmp(str1 + pos, str2, len);
}

ssize_t goto_next(const char *str, char delimiter, size_t len, size_t *hint)
{
    size_t i = 0;
    if (*hint == len)
        return -1;
    for (i = *hint; str[i] != '\0'; i++) {
        if (str[i] == delimiter) {
            *hint = i;
            return i;
        }
    }
    *hint = i;
    return i;
}

bool author_exists(struct project_details *pd, const char *author_name)
{
    size_t hint = 0, start = 0;
    ssize_t len = goto_next(pd->authors.buf, ';', pd->authors.len, &hint);

    while (len > 0) {
        if (match_name(pd->authors.buf, author_name, start, len))
            return true;
        len = goto_next(pd->authors.buf, ';', pd->authors.len, &hint);
        start += len + 1;
    }
    return false;
}

void set_project_author(struct project_details *pd, const char *author_name)
{
    if (author_exists(pd, author_name)) {
        fprintf(stderr, "%s: user is already author.\n", author_name);
        fprintf(stderr, " Try with different name\n");
        return;
    }
    strbuf_addch(&pd->project_name, ';');
    strbuf_addstr(&pd->project_name, author_name);
}

void set_project_description(struct project_details *pd,
                             struct strbuf *description, int append)
{
    int pos = append ? pd->project_desc.len : 0;
    strbuf_insert(&pd->project_desc, pos, description->buf, description->len);
}

void set_project_start_date(struct project_details *pd, struct strbuf *date)
{
    strbuf_reset(&pd->start_date);
    strbuf_addbuf(&pd->start_date, date);
}
