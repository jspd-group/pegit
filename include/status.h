#ifndef STATUS_H_
#define STATUS_H_

#include "util.h"
#include "strbuf.h"

// Data structures used in status.c
struct node {
    char *name;
    int status; // status of the file
    struct node *next;
};

struct d_node {
    struct strbuf name;
    char *parent_dir;
    struct d_node *next;
    struct d_node *previous;
};

#define DELETED 0x8
#define MODIFIED 0x1
#define NEW 0x2

#define S_DELETED 0x800
#define S_MODIFIED 0x100
#define S_NEW 0x200

#define s_is_modified(x) ((x->status == S_DELETED) || \
    (x->status == S_MODIFIED) || (x->status == S_NEW))

extern struct node *root;
extern int count_new, count_modified, count_cached;
// Function declarations of status.c
extern int status(char *);
extern struct node *createnode();
extern void intialise_node(struct node **, char *, int, struct node *);
extern void insert_node(struct node **, struct node **, struct node **);
extern struct d_node *pop();
extern void push(struct dirent *, char *);
extern char *path(struct d_node *);
extern void print_status(struct node *);

extern int status_main(int argc, char* argv[]);
#endif
