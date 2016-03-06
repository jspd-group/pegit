#include "visitor.h"

void scan_dir(struct visitor *v)
{
    struct strbuf name;
    strbuf_init(&name, 1024);
    while (visitor_visit_next_entry(v) == 0) {
        visitor_get_absolute_path(v, &name);
        printf("%s\n", name.buf);
        strbuf_setlen(&name, 0);
    }
}

int main(int argc, char *argv[])
{
    struct visitor v;
    char *path = ".";

    if (argc > 1) path = argv[1];

    visitor_init(&v);
    visitor_visit(&v, path);
    scan_dir(&v);
    return 0;
}
