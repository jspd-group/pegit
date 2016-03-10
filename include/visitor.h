#ifndef VISITOR_H_
#define VISITOR_H_

#include "strbuf.h"
#include "tree.h"

enum { _FILE, _FOLDER, _UNKNOWN };

struct visitor {
    /* path of the current working directory or file */
    struct strbuf path;
    struct dirent *current_entry; /* pointer to the current entry */
    DIR *root;                    /* root directory */
    int entry_type;               /* type of the entry */
    int visited_all; /* tells whether all the entries have been visited */
};

/**
 * Initialise a visitor class. It must be used before using struct visitor.
 */
extern void visitor_init(struct visitor *v);

/**
 * close directory pointed by visitor
 */
extern void visitor_close(struct visitor *v);
/**
 * Visit a given directory. Sets the visitor to the given directory. This
 * must be done before visiting a particular entry in the directory.
 * returns 0 if successful, else -1.
 */
extern int visitor_visit(struct visitor *v, const char *path);

/**
 * Visit the next entry in the current directory pointed by v->path.
 * returns 0, if successfully reached and some entries are remaining.
 * If all the entries have been visited, returns 1. v->current_entry will be
 * NULL after that. returns -1, if failed.
 */
extern int visitor_visit_next_entry(struct visitor *v);

/**
 * Visit a child directory in the current directory.
 * NOTE: The child directory will be one pointed by the current_entry field
 *       in the visitor struct. So be sure that whether the entry_type is
 *       _FOLDER.
 * returns 0 on successful, -1 on failure.
 */
extern int visitor_visit_child_directory(struct visitor *v);

/**
 * Visit the parent directory i.e. `..'.
 * returns 0 if successful, -1 on failure.
 */
extern int visitor_visit_parent(struct visitor *v);

/**
 * Makes folder in the directory path stored in v->path.
 * returns 0 if successful, -1 o failure.
 */
extern int visitor_make_folder(struct visitor *v, const char *name);

/**
 * returns the absolute path of the current entry
 */
static inline void visitor_get_absolute_path(struct visitor *v,
                                             struct strbuf *path)
{
    strbuf_addbuf(path, &v->path);
    strbuf_addch(path, '/');
    strbuf_addstr(path, v->current_entry->d_name);
}

#endif