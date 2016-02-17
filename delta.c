#include "delta.h"
#include <string.h>
#include <unistd.h>

/* arrow types used in delta algorithms */
enum arrow_t : char {
    DELTA_UP,
    DELTA_DOWN,
    DELTA_LEFT,
    DELTA_TILT
};

/** delta_table used to run the main algorithm */
struct delta_table {
    enum arrow_t **table;    /* two dimensional arrow table */
    int *prev;    /* previous table entries */
    int *sol;    /* actual solution entry */
    int row;      /* number of rows */
    int col;        /* number of columns */
};

int delta_table_init(struct delta_table *table, int row, int col)
{
    table->table = (enum arrow_t**)malloc(sizeof(enum arrow_t*) * row);
    if (!table->table) {
        die("Out of memory\n");
        return -1;
    }

    for (int i = 0; i < row; i++) {
        table->table[i] = (enum arrow_t*)malloc(sizeof(enum arrow_t) * col);
        if (!table->table[i]) {
            die("Out of memory\n");
            return -1;
        }
        // memset(table->table[i], DELTA_DOWN, sizeof(enum arrow_t) * col);
    }

    table->prev = (int*)malloc(sizeof(int) * col);
    table->sol = (int*)malloc(sizeof(int) * col);

    if (!table->prev || !table->sol) {
        die("Out of memory");
    }

    memset(table->prev, 0, sizeof(int) * col);
    memset(table->sol, 0, sizeof(int) * col);

    table->row = row;
    table->col = col;
    return 0;
}

void delta_table_free(struct delta_table *table)
{
    while (table->row--) {
        free(table->table[table->row]);
    }
    free(table->table);
    free(table->prev);
    free(table->sol);
}

struct delta_input {
    struct filespec *fs1;
    struct filespec *fs2;

    struct deltafile df1;
    struct deltafile df2;
};


int delta_input_init(struct delta_input *di, struct filespec *fs1,
                            struct filespec *fs2)
{
    di->fs1 = fs1;
    di->fs2 = fs2;

    if (deltafile_init_filespec(&di->df1, fs1) < 0)
        return -1;

    if (deltafile_init_filespec(&di->df2, fs2) < 0)
        return -1;

    return 0;
}

void delta_input_free(struct delta_input *di)
{
    deltafile_free(&di->df1);
    deltafile_free(&di->df2);
}


size_t delta_basic_comparison(struct delta_table *out,
             struct strbuf *a, struct strbuf *b)
{
    int i = 0;
    int j = 0;
    int prev = 0;
    int sol = 0;
    int m = out->col - 1;
    int n = out->row - 1;

    while (++i <= n) {
        while (++j <= m) {
            if (!strbuf_cmp(a[j - 1], b[i - 1])) {
                out->sol[j] = prev + 1;
                out->table[i][j] = DELTA_TILT;
            }
            else if (prev < sol) {
                out->sol[j] = sol;
                out->table[i][j] = DELTA_LEFT;
            }
            else {
                out->sol[j] = out->prev[j];
                out->table[i][j] = DELTA_UP;
            }

            prev = out->prev[j];
            out->prev[j] = out->sol[j];
            sol = out->sol[j];
        }

        prev = out->prev[0];
        sol = out->sol[0];

        j = 0;
    }

    return out->sol[m];
}

struct basic_delta_result {
    size_t insertions;    /* number of insertions */
    size_t deletions;   /* number of deletions */
    size_t common;  /* common number of lines */

    struct strbuf_list common_lines; /* buffer to store the
                                        similar lines */

    struct strbuf_list diff_lines; /* buffer to store the non-similar lines */
};

int delta_backtrace_table(struct basic_delta_result *result,
                          struct delta_table *table,
                          struct strbuf *a,
                          struct strbuf *b)
{
    result->common = table->sol[table->col - 1];

    int i, j;    // index of the current cell
    int **tab = table->table;
    int row = table->row, col = table->col;

    i = row;
    j = col - 1;
    while (i != -1 && j != -1) {
        switch (tab[i][j]) {
        case DELTA_UP:
            result->deletions++;
            strbuf_list_add(&result->diff_lines, b + j, '-', j + 1);
            i--;
            break;

        case DELTA_LEFT:
            result->insertions++;
            strbuf_list_add(&result->diff_lines, a + j, '+', j + 1);
            j--;
            break;

        case DELTA_TILT:
            strbuf_list_add(&result->common_lines, a + j, '~', j + 1);
            j--;
            i--;
            break;

        default:
            die("some error occurred in delta\n");
        }
    }

    return 0;
}