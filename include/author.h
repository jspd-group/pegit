#ifndef AUTHOR_H_
#define AUTHOR_H_

#include "strbuf.h"
#include "util.h"

struct author {
    struct strbuf name;
    struct strbuf email;
};

static inline void author_init(struct author *auth)
{
    strbuf_init(&auth->name, 256);
    strbuf_init(&auth->email, 256);
}

static inline
void author_create(struct author *auth, const char *name,
                    const char *email)
{
    strbuf_addstr(&auth->name, name);
    strbuf_addstr(&auth->email, email);
}

static inline void author_write(struct author *auth, FILE *f)
{
    size_t len = auth->name.len;

    fwrite(&len, sizeof(size_t), 1, f);
    len = fwrite(auth->name.buf, sizeof(char), auth->name.len, f);
    if (len != auth->name.len)
        die("fatal: unable to write\n\t:(\n");
    fwrite(&auth->email.len, sizeof(size_t), 1, f);
    len = fwrite(auth->email.buf, sizeof(char), auth->email.len, f);

    if (len != auth->email.len)
        die("fatal: unable to write\n\t:(\n");
}

static inline void author_read(struct author *auth, FILE *f)
{
    size_t len = 0;

    fread(&len, sizeof(size_t), 1, f);
    if (len > auth->name.alloc) die("fatal: `len' outside `alloc'\n");
    strbuf_fread(&auth->name, len, f);
    fread(&len, sizeof(size_t), 1, f);
    if (len > auth->name.alloc) die("fatal: `len' outside `alloc'\n");
    strbuf_fread(&auth->email, len, f);
}

static inline void author_del(struct author *auth)
{
    strbuf_release(&auth->name);
    strbuf_release(&auth->email);
}

#endif
