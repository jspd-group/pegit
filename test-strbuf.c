#include "strbuf.h"
#include <stdio.h>
#include <stdlib.h>

void print(struct strbuf buffer) {
    printf("%d, %d, %s\n", buffer.alloc, buffer.len, buffer.buf);
}

int main() {
    struct strbuf buffer;
    strbuf_init(&buffer, 0);
    strbuf_grow(&buffer, 100);
    print(buffer);
    strbuf_addchars(&buffer, 'a', 90);
    print(buffer);
    strbuf_insert(&buffer, 40, "prince dhaliwal", 15);
    strbuf_fread(&buffer, 100, stdin);
    print(buffer);
    printf("%d", strbuf_avail(&buffer));
    strbuf_release(&buffer);
    return 0;
}