#ifndef AUTHOR_H_
#define AUTHOR_H_

#include "strbuf.h"
#include "util.h"

struct author {
    struct strbuf name;
    struct strbuf email;
    struct strbuf uid;
    struct strbuf pswd;
};

static inline void author_init(struct author *auth)
{
    strbuf_init(&auth->name, 256);
    strbuf_init(&auth->email, 256);
    strbuf_init(&auth->uid, 10);
    strbuf_init(&auth->pswd, 12);
}

static inline
void author_create(struct author *auth, const char *name,
                    const char *email, const char *uid,
                    const char *pswd)
{
    strbuf_addstr(&auth->name, name);
    strbuf_addstr(&auth->email, email);
    strbuf_addstr(&auth->uid, uid);
    strbuf_addstr(&auth->pswd, pswd);
}

#endif
