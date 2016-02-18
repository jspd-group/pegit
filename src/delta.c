#include "delta.h"
#include "strbuf-list.h"
#include <string.h>
#include <unistd.h>


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


int delta_input_init(struct delta_input *di, struct filespec *fs1,
                            struct filespec *fs2)
{
    di->fs1 = fs1;
    di->fs2 = fs2;

    if (deltafile_init_filespec(&di->df1, fs1, DELIM) < 0)
        return -1;

    if (deltafile_init_filespec(&di->df2, fs2, DELIM) < 0)
        return -1;

    return 0;
}

void delta_input_free(struct delta_input *di)
{
    deltafile_free(&di->df1);
    deltafile_free(&di->df2);
}

void print_table(const char *str, int *t, int len)
{
    printf("%s: ", str);
    for (int i = 0; i <= len; i++)
    {
        printf("%d ", t[i]);
    }
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
// #if DEBUG
//         print_table("prev table", out->prev, m);
// #endif

        while (++j <= m) {
            if (!strbuf_cmp(a + i - 1, b + j - 1)) {
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
#if DEBUG
        print_table("", out->prev, m);
        printf("\t%d, %d", i, j);
        printf("\n");
#endif
        prev = out->prev[0];
        sol = out->sol[0];

        j = 0;
    }

    return out->sol[m];
}

void basic_delta_result_init(struct basic_delta_result *bdr)
{
    bdr->insertions = 0;
    bdr->deletions = 0;
    bdr->common = 0;
    strbuf_list_init(&bdr->common_lines);
    strbuf_list_init(&bdr->diff_lines);
}

int delta_backtrace_table(struct basic_delta_result *result,
                          struct delta_table *table,
                          struct strbuf *a,
                          struct strbuf *b)
{
    result->common = table->sol[table->col - 1];

    int i, j;    // index of the current cell
    enum arrow_t **tab = table->table;
    int row = table->row, col = table->col;

    i = row - 2;
    j = col - 2;
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
#if DEBUG
        printf("I: %d, D: %d, C: %d\n", result->insertions, result->deletions);
#endif

    }

    return 0;
}