#ifndef STRBUF_LIST_H_
#define STRBUF_LIST_H_

#include "strbuf.h"
#include "util.h"

struct strbuf_list_node {
    struct strbuf buf;             /* element */
    char sign;                     /* '+' or '-' */
    size_t index;                  /* index of the line */
    struct strbuf_list_node *next; /* next pointer */
};

struct strbuf_list {
    struct strbuf_list_node *head;
    struct strbuf_list_node *last;
    size_t size;
};

static int strbuf_list_init(struct strbuf_list *sbl)
{
    sbl->head = MALLOC(struct strbuf_list_node, 1);

    if (!sbl->head) die("Out of memory\n");

    sbl->head->next = NULL;
    sbl->last = sbl->head;
    sbl->size = 0;
    return 0;
}

static int strbuf_list_add(struct strbuf_list *sbl, const void *buf,
                           size_t size, char sign, size_t index)
{
    struct strbuf_list_node *node = MALLOC(struct strbuf_list_node, 1);

    if (!node) die("Out of memory\n");

    strbuf_init(&node->buf, size);
    strbuf_add(&node->buf, buf, size);
    node->next = sbl->head->next;
    sbl->head->next = node;
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
    } while ((node = sbl->head));
}

static void strbuf_list_append(struct strbuf_list *sbl, struct strbuf_list *sec)
{
    sbl->last->next = sec->head->next;
    sbl->last = sec->last;
    sec->head->next = NULL;
    sec->last = sec->head;
}

#endif /* STRBUF_LIST_H_ */