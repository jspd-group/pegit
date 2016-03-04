#include "delta-file.h"

int main(int argc, char *argv[]) {
    struct deltafile df;
    char *file = NULL;

    if (argc > 1)
        file = argv[1];
    else {
        printf("Please supply a file\n");
        exit(0);
    }
    struct filespec fs;
    if (filespec_init(&fs, file, "r") < 0){
        return -1;
    }

    deltafile_init_filespec(&df, &fs, '\n');

    printf("%d\n", df.size);
    for (int i = 0; i < df.size; i++) {
        printf("%d, ", df.arr[i]);
    }
    getc(stdin);
    deltafile_free(&df);
    filespec_free(&fs);
    getc(stdin);
    return 0;
}