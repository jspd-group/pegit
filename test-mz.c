#include "strbuf.h"
#include "util.h"

void print(struct strbuf buffer) {
    printf("%d byte(s)\n", buffer.len);
}

// returns the number of characters in the file
static size_t file_length(FILE *file) {
    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    return length;
}

// create a sample buffer
void make_buffer(struct strbuf *buffer) {
    strbuf_init(buffer, 11102809);
    strbuf_addchars(buffer, 'a', 111028439);
}


// an example function to show compression
void test_file_compression() {

    // create two buffers one for source data and other for
    // storing the result of the compression
    struct strbuf src, dest;

    // ### NOTE: this is required step and buffers must be initialised
    // before using them
    strbuf_init(&src, 0);
    strbuf_init(&dest, 0);

    // read the data to src buffer
    FILE *file = fopen("docs/readme.txt", "r");

    // use this function for reading the data
    strbuf_fread(&src, file_length(file), file);
    printf("%d Characters before compression\n", src.len);
    fclose(file);

    // compress the data and store the result to the dest buffer
    // ### NOTE: you can also use another function ie
    //
    //          compress(src, dest, an integer between 0 - 9);
    //
    // integer should lie between 0 to 9, 0 for fast but no compression
    // 9 for slow but best compression, if you don't know which to use
    // use compress_default
    compress_default(&src, &dest);

    // open the file where you would store your data
    file = fopen("docs/temp.xxx", "wb");

    // write the compressed data buffer to the file
    fwrite(dest.buf, sizeof(char), dest.len, file);
    printf("%d Characters after compression\n", dest.len);

    // cleanup
    fclose(file);
    strbuf_release(&src);
    strbuf_release(&dest);
}


// an example function showing how decompression is done
void test_file_decompression() {
    // create two buffers one for source data and other
    // for storing the decompression result
    struct strbuf src, dest;

    //initialise the buffers ### important
    strbuf_init(&src, 0);
    strbuf_init(&dest, 0);

    // open the file
    FILE *file = fopen("docs/temp.xxx", "rb");

    // use this function to read from file specifying the
    // number of characters to read
    strbuf_fread(&src, file_length(file), file);
    printf("\n%d Characters read\n", src.len, src.buf);

    // uncompress the data read
    // ### your data should be valid otherwise it will stuck here
    uncompress(&src, &dest);
    printf("%s\n", dest.buf);

    // cleanup
    fclose(file);
    strbuf_release(&src);
    strbuf_release(&dest);
}

int main() {
    test_file_compression();
    test_file_decompression();


    // another example

    // create buffers
    struct strbuf src, dest, temp;

    // initialise the buffers
    strbuf_init(&src, 0);
    strbuf_init(&dest, 0);
    strbuf_init(&temp, 0);

    // put the data into the source buffer
    make_buffer(&src);

    printf("Before compression: ");
    printf("\nSource: ");
    print(src);
    printf("Destination: ");
    print(dest);


    // compress the src to dest
    compress_default(&src, &dest);

    printf("After compression: ");
    printf("\nSource: ");
    print(src);
    printf("Destination: ");
    print(dest);

    // uncompress the compressed data to temp
    uncompress(&dest, &temp);


    printf("After dempression: ");
    printf("\nSource: ");
    print(dest);
    printf("Destination: ");
    print(temp);

    // check the validity of the source and uncompressed buffers
    int ret = strbuf_cmp(&src, &temp);
    printf("%s\n", ret ? "Failed" : "OK");

    // cleanup
    strbuf_release(&src);
    strbuf_release(&dest);
    strbuf_release(&temp);
    return 0;
}
