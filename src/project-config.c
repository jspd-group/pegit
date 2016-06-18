#include "global.h"
#include "strbuf.h"

void project_details_init(struct project_details *pd)
{
    strbuf_init(&pd->project_name, 256);
    strbuf_init(&pd->authors, 1024);
    strbuf_init(&pd->project_desc, 1024);
    strbuf_init(&pd->start_date, 256);
}

void project_details_free(struct project_details *pd)
{
    strbuf_release(&pd->project_name);
    strbuf_release(&pd->authors);
    strbuf_release(&pd->start_date);
    strbuf_release(&pd->project_desc);
}

void set_project_name(struct project_details *pd, const char *name)
{
    strbuf_setlen(&pd->project_name, 0);
    strbuf_addstr(&pd->project_name, name);
}

static inline bool match_name(const char *str1, const char *str2, int pos,
                              int len)
{
    return !strncmp(str1 + pos, str2, len);
}

ssize_t goto_next(const char *str, char delimiter, size_t len, size_t *hint)
{
    size_t i = 0;
    if (*hint == len)
        return -1;
    for (i = *hint; str[i] != '\0'; i++) {
        if (str[i] == delimiter) {
            *hint = i;
            return i;
        }
    }
    *hint = i;
    return i;
}

bool author_exists(struct project_details *pd, const char *author_name)
{
    size_t hint = 0, start = 0;
    ssize_t len = goto_next(pd->authors.buf, ';', pd->authors.len, &hint);

    while (len > 0) {
        if (match_name(pd->authors.buf, author_name, start, len))
            return true;
        len = goto_next(pd->authors.buf, ';', pd->authors.len, &hint);
        start += len + 1;
    }
    return false;
}

void set_project_author(struct project_details *pd, const char *author_name)
{
    if (author_exists(pd, author_name)) {
        fprintf(stderr, "%s: user is already author.\n", author_name);
        fprintf(stderr, " Try with different name\n");
        return;
    }
    strbuf_addch(&pd->project_name, ';');
    strbuf_addstr(&pd->project_name, author_name);
}

void set_project_description(struct project_details *pd,
                             struct strbuf *description, int append)
{
    int pos = append ? pd->project_desc.len : 0;
    strbuf_insert(&pd->project_desc, pos, description->buf, description->len);
}

void set_project_start_date(struct project_details *pd, struct strbuf *date)
{
    strbuf_reset(&pd->start_date);
    strbuf_addbuf(&pd->start_date, date);
}

struct config_list *new_config_list_node()
{
    struct config_list *new = malloc(sizeof(struct config_list));
    strbuf_init(&new->key, 0);
    strbuf_init(&new->value, 0);
    new->next = NULL;
    return new;
}

void config_list_insert(struct config_list **head, struct config_list *new)
{
    new->next = *head;
    *head = new;
}

void read_config_file(struct peg_env *env)
{
    int config_file = open(env->peg_config_filepath, O_RDONLY);

    if (config_file == -1) {
        printf("no config file found\n");
        die("%s\n", strerror(errno));
    }
    strbuf_read(&env->cache, config_file, 1024);
}

enum token_type { INVALID, STRING, ASSIGN, NUMBER, COMMENT };

struct lex {
    const char *str;
    size_t pos;
    size_t len;
    enum token_type curr;
};

struct parser_state {
    bool key;
    bool value;
};

int advance(struct lex *lex, int go)
{
    if (!go)
        return 0;
    while (lex->pos < lex->len &&
           (lex->str[lex->pos] == ' ' || lex->str[lex->pos] == '\n' ||
            lex->str[lex->pos] == '\t' || lex->str[lex->pos] == '\n'))
        lex->pos++;

    if (lex->pos >= lex->len) {
        return -1;
    }
    if (isdigit(lex->str[lex->pos])) {
        lex->curr = NUMBER;
        return NUMBER;
    } else if (isalpha(lex->str[lex->pos])) {
        lex->curr = STRING;
        return STRING;
    } else if (lex->str[lex->pos] == '=') {
        lex->curr = ASSIGN;
        return ASSIGN;
    } else if (lex->str[lex->pos] == '#') {
        lex->curr = COMMENT;
        return COMMENT;
    }
    return 0;
}

void consume(struct lex *lex, size_t n) { lex->pos += n; }

void consume_comment(struct lex *lex)
{
    while (lex->str[lex->pos] != '\n') {
        lex->pos++;
    }
}

int read_number(struct config_list *node, struct lex *lex)
{
    size_t len = lex->pos;

    while (len < lex->len && isdigit(lex->str[len]))
        len++;
    strbuf_add(&node->value, lex->str + lex->pos, len - lex->pos);
    node->number = strtol(node->value.buf, NULL, 10);
    consume(lex, len - lex->pos);
    return 0;
}

int parse_string(struct config_list *node, struct lex *lex, int key)
{
    size_t len = lex->pos;

    if (key)
        while (len < lex->len &&
               (isalnum(lex->str[len]) || lex->str[len] == '-')) {
            len++;
        }
    else {
        while (len < lex->len && (lex->str[len] != '\n')) {
            len++;
        }
    }
    key ? strbuf_add(&node->key, lex->str + lex->pos, len - lex->pos)
        : strbuf_add(&node->value, lex->str + lex->pos, len - lex->pos);
    consume(lex, len - lex->pos);
    return 0;
}

int parse_config_file(struct peg_env *env)
{
    struct config_list *node = NULL;
    struct lex lex = {NULL, 0, 0, INVALID};
    struct parser_state state = {0, 0};
    int should_go = 1;

    lex.str = env->cache.buf;
    lex.len = env->cache.len;
    lex.pos = 0;
    while (advance(&lex, should_go) >= 0) {
        if (!state.key)
            node = new_config_list_node();
        if (state.key && state.value) {
            config_list_insert(&env->list, node);
            state.key = 0;
            state.value = 0;
            should_go = 1;
            continue;
        }
        switch (lex.curr) {
        case INVALID:
            die("stray found in config file.\n");

        case NUMBER:
            if (!state.key) {
                die("error while reading config file.\n");
            }
            if (!read_number(node, &lex))
                die("invalid value for `%s'\n", node->key.buf);
            state.value = 1;
            should_go = 0;
            continue;

        case STRING:
            if (state.key) {
                parse_string(node, &lex, 0);
                state.value = 1;
                should_go = 0;
            } else {
                parse_string(node, &lex, 1);
                state.key = 1;
            }
            continue;

        case ASSIGN:
            consume(&lex, 1);
            continue;

        case COMMENT:
            consume_comment(&lex);
            continue;
        }
    }

    return 0;
}

int create_environment(struct peg_env *env)
{
    char *userprofile;
    struct strbuf buf = STRBUF_INIT;

    env->peg_state = S_STARTUP;
    userprofile = getenv(USERPROFILE);
    strbuf_addstr(&buf, userprofile);
    strbuf_addstr(&buf, "/.pegconfigure");
    env->peg_config_filepath = buf.buf;
    env->owner = NULL;
    strbuf_init(&env->cache, 0);
    env->owner_email = NULL;
    env->list = NULL;
    return 0;
}

struct config_list *get_environment_value(struct peg_env *env, const char *key)
{
    struct config_list *node = env->list;

    while (node) {
        if (!strcmp(key, node->key.buf))
            return node;
        node = node->next;
    }
    return NULL;
}
