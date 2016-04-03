#ifndef GLOBAL_H_
#define GLOBAL_H_

#include "strbuf.h"
#include "author.h"

enum {
    S_STARTUP,
    S_END,
    S_CRITICAL,
    S_NORMAL,
    S_QUITABLE
};

struct config_list {
    struct strbuf key;
    struct strbuf value;
    ssize_t number;
    struct config_list *next;
};

struct peg_env {
    int peg_state;
    char *peg_config_filepath;
    char *owner;
    struct config_list *list;
    struct strbuf cache;
    char *owner_email;
};

extern struct peg_env environment;

/**
 * this struct represents the project's details
 */
struct project_details {
    struct strbuf project_name;
    struct strbuf authors;
    struct strbuf project_desc;
    struct strbuf start_date;
};

/**
 * Initialise the project_details structure
 */
extern void project_details_init(struct project_details *pd);

/* set the name of the project */
extern void set_project_name(struct project_details *pd, const char *name);

/* set the author name or add a new author */
extern void
set_project_author(struct project_details *pd, const char *author_name);

/* set the project description or append it to the old */
extern void
set_project_description(struct project_details *pd,
                             struct strbuf *description, int append);

/* set the project start date */
extern void
set_project_start_date(struct project_details *pd, struct strbuf *date);


extern struct config_list *get_environment_value(struct peg_env *env,
    const char *key);

extern int parse_config_file(struct peg_env *env);

extern void read_config_file(struct peg_env *env);

static inline void get_global_author(struct author *a)
{
    strbuf_addstr(&a->name, environment.owner);
    strbuf_addstr(&a->email, environment.owner_email);
}

#endif
