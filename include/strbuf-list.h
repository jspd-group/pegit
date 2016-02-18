#ifndef STRBUF_LIST_H_
#define STRBUF_LIST_H_

#include "strbuf.h"
#include "util.h"

struct strbuf_list_node {
    struct strbuf buf;        /* element */
    char sign;        /* '+' or '-' */
    size_t index;    /* index of the line */
    struct strbuf_list_node *next; /* next pointer */
};

struct strbuf_list {
    struct strbuf_list_node *head;
    struct strbuf_list_node *last;
    size_t size;
};

static int strbuf_list_init(struct strbuf_list *sbl)
{
    sbl->head = (struct strbuf_list_node*)malloc(sizeof(struct strbuf_list_node));

    if (!sbl->head || !sbl->last)
        die("Out of memory\n");

    sbl->head->next = NULL;
    sbl->last = sbl->head;
    sbl->size = 0;
    return 0;
}

static int strbuf_list_add(struct strbuf_list *sbl,
                           struct strbuf *buf, char sign, size_t index)
{
    struct strbuf_list_node *node = (struct strbuf_list_node*)
                                malloc(sizeof(struct strbuf_list_node));

    if (!node)
        die("Out of memory\n");

    strbuf_init(&node->buf, buf->len);
    strbuf_addbuf(&node->buf, buf);

    node->next = NULL;
    sbl->last->next = node;
    sbl->last = sbl->last->next;
    sbl->size += 1;
    node->sign = sign;
    node->index = index;

    return 0;
}

static void strbuf_list_free(struct strbuf_list *sbl)
{
    struct strbuf_list_node *node = sbl->head;
    do {
        sbl->head = sbl->head->next;
        free(node);
    } while (node = sbl->head);
}

#endif /* STRBUF_LIST_H_ */