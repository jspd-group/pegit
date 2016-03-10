#ifndef PROJECT_CONFIG_H_
#define PROJECT_CONFIG_H_

#include "util.h"
#include "strbuf.h"

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

#endif
