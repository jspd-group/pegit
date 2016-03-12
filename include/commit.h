#ifndef COMMIT_H_
#define COMMIT_H_

#include "cache.h"
#include "author.h"
#include "util.h"
#include "sha1-inl.h"
#include "timestamp.h"
#include "delta.h"
#include "index.h"

struct commit {
    short sha1[HASH_SIZE];    /* sha1 hash */
    struct author *auth;    /* author of the commit */
    struct strbuf cmt_msg; /* commit message */
    struct strbuf cmt_desc; /* commit description */
    struct timestamp stamp;    /* time of commit */
    size_t commit_index;    /* pointers to the index file */
    size_t commit_length; /* length of the commits */
};

struct commit_list {
    size_t count;
    struct commit *item;
    struct commit_list *next;
};

struct index_list {
    struct index *idx;
    struct index_list *next;
};

typedef int(commit_fn)(struct commit *);
typedef bool(commit_match_fn)(struct commit *);

/**
 * Generate a new commit.
 */
extern int generate_new_commit(struct strbuf *cmt, struct strbuf *det);

extern int commit(int argc, char *argv[]);
#endif
