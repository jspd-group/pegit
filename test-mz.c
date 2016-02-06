#include <stdio.h>
#include "strbuf.h"

void print(struct strbuf buffer) {
    printf("%d, %d, ", buffer.alloc, buffer.len);
    for (int i = 0; i < buffer.len; i++) {
        printf("%c", buffer.buf[i]);
    }
    printf("\n");
}

int main() {
    struct strbuf src, dest, temp;
    strbuf_init(&src, 0);
    strbuf_init(&dest, 0);
    strbuf_add(&src, "HelloHelloHelloHelloHellovnkdflvmdlmvlsd", 40);
    print(src);
    print(dest);
    // compress the src
    compress_default(&src, &dest);

    print(src);
    print(dest);

    strbuf_release(&src);
    strbuf_init(&temp, 0);
    uncompress(&dest, &temp);
    print(temp);
    print(dest);

    return 0;
}