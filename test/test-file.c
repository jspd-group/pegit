#include "file.h"



int main() {
    struct filespec fs;
    printf("Initializing fs\n");
    filespec_init(&fs, "./ReadMe.md", "r");

    struct strbuf buf;
    strbuf_init(&buf, 0);
    printf("Reading %s\n", fs.fname.buf);
    filespec_read_unsafe(&fs, &buf, 0);


    printf("Printing fs\n");
    strbuf_print(&buf);

    printf("Printing Hash\n");
    char sha[20] = { '\0' };

    filespec_sha1(&fs, sha);

#if DEBUG
    for (int i = 0; i < HASH_SIZE; i++)
        printf("%02x", (uint8_t)sha[i]);
#endif
    printf("\n");
    return 0;
}