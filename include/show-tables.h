#ifndef SHOW_TABLES_H_
#define SHOW_TABLES_H_

#include "commit.h"
#include "stage.h"

extern void show_commit_node(struct commit_list *node, size_t i);

extern void print_html_page();
extern void show_cache_table_html();

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

static void print_head_commit_short()
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
    printf("%s %s\n", node->item->sha1str, node->item->cmt_msg.buf);
}

static int show_tables(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--cached") || !strcmp(argv[i], "--staged") ||
            !strcmp(argv[i], "--temp"))
            show_cache_table();
        else if (!strcmp(argv[i], "--commit") ||
                 !strcmp(argv[i], "--permanent"))
            show_commit_table();
        else if (!strcmp(argv[i], "--commit-count") || !strcmp(argv[i], "-cc"))
            show_commit_count();
        else if (!strcmp(argv[i], "--head") || !strcmp(argv[i], "--HEAD"))
            print_head_commit();
        else if (!strcmp(argv[i], "--html-commit") || !strcmp(argv[i], "-hc"))
            print_html_page();
        else if (!strcmp(argv[i], "--html-cached") || !strcmp(argv[i], "-hcc"))
            show_cache_table_html();
        else if (!strcmp(argv[i], "--head-short") || !strcmp(argv[i], "-hs"))
            print_head_commit_short();
    }
    return 0;
}

#endif
