#ifndef SHOW_TABLES_H_
#define SHOW_TABLES_H_

#include "stage.h"
#include "commit.h"

extern void show_commit_node(struct commit_list *node, size_t i);

static void print_head_commit()
{
    struct commit_list *cl, *node;
    size_t i = 0;

    make_commit_list(&cl);
    node = cl;

    while (node) {
        if (node->item->flags & HEAD_FLAG) {
            break;
        }
        node = node->next;
        i++;
    }
    if (!node)
        die("no head commit found.\n");
    show_commit_node(node, i);
}

static int show_tables(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--cached") || !strcmp(argv[i], "--staged")
            || !strcmp(argv[i], "--temp"))
            show_cache_table();
        else if (!strcmp(argv[i], "--commit") || !strcmp(argv[i], "--permanent"))
            show_commit_table();
        else if (!strcmp(argv[i], "--commit-count") || !strcmp(argv[i], "-cc"))
            show_commit_count();
        else if (!strcmp(argv[i], "--head") || !strcmp(argv[i], "--HEAD"))
            print_head_commit();
    }
    return 0;
}

#endif
