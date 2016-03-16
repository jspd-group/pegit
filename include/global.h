#ifndef GLOBAL_H_
#define GLOBAL_H_

#include "strbuf.h"
#include "author.h"

static inline void get_global_author(struct author *a)
{
    strbuf_addstr(&a->name, "dummy");
    strbuf_addstr(&a->email, "dummy@dummy.com");
}

#endif
