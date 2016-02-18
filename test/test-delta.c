#include "delta.h"

#define TEST_DATA "test/data/"

void print_result(struct delta_table *table)
{
    printf("%d\n", table->sol[table->col - 2]);
}

void print_input(struct delta_input *input)
{
    for (int i = 0; i < input->df2.size; i++) {
        printf("%s", input->df2.arr[i].buf);
    }
    printf("\n");
    for (int i = 0; i < input->df1.size; i++) {
        printf("%s", input->df1.arr[i].buf);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    /* tests for delta */
    struct delta_input input;
    struct filespec a, b;
    struct delta_table table;
    struct basic_delta_result result;

    char *f1 = TEST_DATA"a.txt";
    char *f2 = TEST_DATA"a.txt";
    if (argc > 1) {
        printf("Argc\n");
        f1 = argv[1];
        f2 = argv[2];
    }

    filespec_init(&a, f1, "r");
    filespec_init(&b, f2, "r");
    printf("Done\n");
    delta_input_init(&input, &a, &b);

    delta_table_init(&table, input.df1.size + 1, input.df2.size + 1);
    printf("%d, %d\n", input.df1.size, input.df2.size);
    print_input(&input);
    basic_delta_result_init(&result);

    delta_basic_comparison(&table, input.df1.arr, input.df2.arr);

    printf("Backtracking table\n");
    //delta_backtrace_table(&result, &table, input.df1.arr, input.df2.arr);

    print_result(&table);
    return 0;
}