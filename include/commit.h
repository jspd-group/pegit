#ifndef COMMIT_H_
#define COMMIT_H_

#include "cache.h"
#include "author.h"
#include "util.h"
#include "sha1-inl.h"
#include "timestamp.h"
#include "delta.h"
#include "pack.h"
#include "index.h"
#include "status.h"

struct commit {
    int16_t flags; /* various commit flags */
    char sha1[HASH_SIZE];    /* sha1 hash */
    char tag[TAG_SIZE]; /* user defined tag */
    char sha1str[SHA_STR_SIZE + 1];
    struct author *auth;    /* author of the commit */
    struct strbuf cmt_msg; /* commit message */
    struct strbuf cmt_desc; /* commit description */
    struct timestamp stamp;    /* time of commit */
    size_t commit_index;    /* pointers to the index file */
    size_t commit_length; /* length of the commits */
};

#define TAG_FLAG 0x1
#define HEAD_FLAG 0x16

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
 *     `cmt' : represents the commit message, which should not be empty
 *     `det' : represents the commit description, which can be empty
 */
extern int generate_new_commit(struct strbuf *cmt, struct strbuf *det,
    char tag[TAG_SIZE], int16_t flags);

/**
 * Kind of main function for generating a new commit.
 *    `argc' : size of argv array
 *    `argv' : array representing the options passed to this function
 *
 *  various options include:
 *        -m, --message MESSAGE : this option is followed by message string.
 *        -d, --desc DESCRIPTION: similar to -m
 *        log                   : logs all the commits
 */
extern int commit(int argc, char *argv[]);

/**
 * Finds the file from the head commit (yet not a better way!).
 * Points to be noted:
 *    Slow,
 *    More memory,
 *    don't call it more and more.
 */
extern bool find_file_from_head_commit(const char *name, struct strbuf *buf);

/**
 * Makes a commit list from the `commit.idx' file in the commit folder.
 */
extern size_t make_commit_list(struct commit_list **head);

/**
 * Deletes the commit_list: the one made by the make_commit_list function
 */
extern void commit_list_del(struct commit_list **head);

extern struct commit *find_commit_hash(struct commit_list *cl,
    char sha1[HASH_SIZE]);

extern struct commit *find_commit_tag(struct commit_list *cl,
    const char *tag);

extern struct commit *find_commit_hash_compat(struct commit_list *cl,
    char *sha1, size_t len);

extern void make_index_list_from_commit(struct commit *node,
    struct index_list **head);

extern struct index *find_file_index_list(struct index_list *head,
    const char *file);

extern struct index_list *get_head_commit_list(struct commit_list *head);
extern struct commit *get_head_commit(struct commit_list *cl);

static inline struct commit *get_master_commit(struct commit_list *cl)
{
    struct commit_list *node, *prev;
    node = cl;
    prev = node;
    while (node) {
        prev = node;
        node = node->next;
    }
    return prev->item;
}

extern int checkout(int argc, char *argv[]);

/**
 * Give a tag to a particular commit if supplied with 'sha1' as NULL
 * the tag will be given to the latest commit.
 */
extern void set_tag(const char *sha1, size_t len, const char *tag);

extern void print_commits();

extern void list_tags();

extern void show_commit_table();
extern void show_commit_count();
extern void revert_files_hard();
extern void print_humanised_bytes(off_t bytes);
extern void reset_flags(struct index_list *il);
#endif
