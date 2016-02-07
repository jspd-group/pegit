#include "strbuf.h"
#include "util.h"

void print(struct strbuf buffer) {
    printf("%d byte(s)\n", buffer.len);
}


// read the files to a C string
// ### string is malloced before writing to it
// user is responsible to read the file
static size_t file_length(FILE *file) {
    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    return length;
}

void make_buffer(struct strbuf *buffer) {
    strbuf_init(buffer, 11102809);
    strbuf_addchars(buffer, 'a', 111028439);
}

void test_file_compression() {
    struct strbuf f, dest;
    strbuf_init(&f, 0);
    FILE *fil = fopen("docs/ReadMe.txt", "r");
    strbuf_fread(&f, file_length(fil), fil);
    strbuf_init(&dest, 0);
    printf("%s\n", f.buf);

    compress_default(&f, &dest);

    FILE *file = fopen("docs/temp.xxx", "wb");
    fwrite(&dest.len, sizeof(size_t), 1, file);
    fwrite(dest.buf, sizeof(char), dest.len, file);
    fclose(file);
    strbuf_release(&f);
    strbuf_release(&dest);
}

void test_file_decompression() {
    struct strbuf src, dest;
    strbuf_init(&src, 0);
    strbuf_init(&dest, 0);
    FILE *file = fopen("docs/temp.xxx", "rb");
    strbuf_fread(&src, file_length(file), file);
    printf("%s\n", src.buf);
    uncompress(&src, &dest);
    printf("%s\n", dest.buf);
}

int main() {
    // test_file_compression();
    // test_file_decompression();

    struct strbuf src, dest, temp;
    strbuf_init(&src, 0);
    strbuf_init(&dest, 0);
    make_buffer(&src);

    printf("Before compression: ");
    printf("\nSource: ");
    print(src);
    printf("Destination: ");
    print(dest);
    // compress the src
    compress_default(&src, &dest);

    printf("After compression: ");
    printf("\nSource: ");
    print(src);
    printf("Destination: ");
    print(dest);

    strbuf_init(&temp, 0);
    uncompress(&dest, &temp);


    printf("After dempression: ");
    printf("\nSource: ");
    print(dest);
    printf("Destination: ");
    print(temp);

    int ret = strbuf_cmp(&src, &temp);
    printf("%s\n", ret ? "Failed" : "OK");
    return 0;
}