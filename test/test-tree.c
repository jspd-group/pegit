#include "strbuf.h"
#include "tree.h"

int print(const char *file_name)
{
    printf("%s\n", file_name);
    return 0;
}

int main(int argc, char *argv[])
{
    char *dir;
    if (argc < 2) {
        dir = ".";
    }
    else {
        dir = argv[1];
    }
    struct strbuf buf = STRBUF_INIT;

    strbuf_addstr(&buf, dir);
    int ret = for_each_file_in_directory_recurse(buf.buf, print);
    printf("%d\n", ret);
    return ret;
}
