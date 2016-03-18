#include "delta.h"

#ifndef _MSC_VER
#define TEST_DATA "test/data/"
#else
#define TEST_DATA "./"
#endif

void print_result(struct delta_table *table)
{
    printf("%d\n", table->sol[table->col]);
}

void print_input(struct delta_input *input)
{
 /*   for (int i = 0; i < input->df2.size; i++) {
        printf("%s", input->df2.arr[i].buf);
    }
    printf("\n");
    for (int i = 0; i < input->df1.size; i++) {
        printf("%s", input->df1.arr[i].buf);
    }
    printf("\n");*/
}

void print_delta_result(struct strbuf_list *list)
{
    struct strbuf_list_node *node;
    node = list->head->next;
    while (node != NULL) {
        printf("%c\t", node->sign);
        printf("%s", node->buf.buf);

        node = node->next;
    }
}

int main(int argc, char *argv[]) {
    if (strcmp(argv[1], "--old")) {
        delta_main(argc, argv);
        return 0;
    }
    /* tests for delta */
    struct delta_input input;
    struct filespec a, b;
    struct delta_table table;
    struct basic_delta_result result;

    char *f1 = TEST_DATA"a.txt";
    char *f2 = TEST_DATA"b.txt";

    filespec_init(&a, f1, "r");
    filespec_init(&b, f2, "r");
    printf("Done\n");
    delta_input_init(&input, &a, &b);

    delta_table_init(&table, input.df1.size, input.df2.size);
    printf("%lld, %lld\n", input.df1.size, input.df2.size);
    // print_input(&input);
    basic_delta_result_init(&result, &input);

    delta_basic_comparison_m(&table, &input.df1, &input.df2);

    printf("Backtracking table\n");
    delta_backtrace_table(&result, &table, &input.df1, &input.df2);

    struct strbuf stat = STRBUF_INIT;
    delta_stat(&result, &stat);
    printf("%s\n", stat.buf);

    return 0;
}
