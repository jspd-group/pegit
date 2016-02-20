#ifndef DELTA_H_
#define DELTA_H_

#include "delta-file.h"
#include "strbuf-list.h"

#define DELIM '\n'

/* arrow types used in delta algorithms */
enum arrow_t {
    DELTA_UP,
    DELTA_DOWN,
    DELTA_LEFT,
    DELTA_TILT
};


/**
 * delta_table used to run the main algorithm
 */
struct delta_table {
    enum arrow_t **table;    /* two dimensional arrow table */
    int *prev;    /* previous table entries */
    int *sol;    /* actual solution entry */
    int row;      /* number of rows */
    int col;        /* number of columns */
};


struct delta_input {
    struct filespec *fs1;
    struct filespec *fs2;

    struct deltafile df1;
    struct deltafile df2;
};

struct basic_delta_result {
    size_t insertions;    /* number of insertions */
    size_t deletions;   /* number of deletions */
    size_t common;  /* common number of lines */

    struct strbuf_list common_lines; /* buffer to store the
                                        similar lines */

    struct strbuf_list diff_lines; /* buffer to store the non-similar lines */
};


extern int delta_table_init(struct delta_table *table, int row, int col);
extern void delta_table_free(struct delta_table *table);
extern int delta_input_init(struct delta_input *di, struct filespec *fs1,
                            struct filespec *fs2);
extern void delta_input_free(struct delta_input *di);
extern size_t delta_basic_comparison(struct delta_table *out,
             struct strbuf *a, struct strbuf *b);
extern int delta_backtrace_table(struct basic_delta_result *result,
                          struct delta_table *table,
                          struct strbuf *a,
                          struct strbuf *b);
extern void basic_delta_result_init(struct basic_delta_result *bdr);

extern size_t delta_basic_comparison_m(struct delta_table *out,
struct deltafile *af, struct deltafile *bf);

#define DELIM '\n'
#endif