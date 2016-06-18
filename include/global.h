#ifndef GLOBAL_H_
#define GLOBAL_H_

#include "author.h"
#include "sha1-inl.h"
#include "strbuf.h"

enum { S_STARTUP, S_END, S_CRITICAL, S_NORMAL, S_QUITABLE };

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
    bool pswd;
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
extern void set_project_author(struct project_details *pd,
                               const char *author_name);

/* set the project description or append it to the old */
extern void set_project_description(struct project_details *pd,
                                    struct strbuf *description, int append);

/* set the project start date */
extern void set_project_start_date(struct project_details *pd,
                                   struct strbuf *date);

extern struct config_list *get_environment_value(struct peg_env *env,
                                                 const char *key);

extern int parse_config_file(struct peg_env *env);

extern void read_config_file(struct peg_env *env);

static inline void get_global_author(struct author *a)
{
    strbuf_addstr(&a->name, environment.owner);
    strbuf_addstr(&a->email, environment.owner_email);
}

static bool validate_user(const char *pswd)
{
    struct config_list *node;
    struct strbuf sha = STRBUF_INIT;
    char sha1[20];

    node = get_environment_value(&environment, "password");

    if (!node) {
        return true;
    }

    return !strcmp(pswd, node->value.buf);
    strbuf_addstr(&sha, pswd);
    strtosha1(&sha, sha1);
    strbuf_release(&sha);
    strbuf_init(&sha, 41);
    for (int i = 0; i < 20; i++) {
        sprintf(sha.buf + sha.len, "%02x", (uint8_t)sha1[i]);
        sha.len += 2;
    }
    sha.buf[--sha.len] = '\0';

    return !strbuf_cmp(&sha, &node->value);
}

static void vud()
{
    char pswd[100];
    if (environment.pswd) {
        printf("Enter password for \"" CYAN "%s" RESET "\": ",
               environment.owner);
        fgets(pswd, 100, stdin);
        while (!validate_user(pswd)) {
            printf("Entered password doesn't match, try again: ");
            fgets(pswd, 100, stdin);
        }
    }
}

extern int create_environment(struct peg_env *env);

#endif
