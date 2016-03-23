#ifndef SHA1_INL_H_
#define SHA1_INL_H_

#include "util.h"
#include "../sha1/sha1.h"
#include "strbuf.h"

#define peg_sha_ctx SHA_CTX
#define sha1_init SHA1_Init
#define sha1_update SHA1_Update
#define sha1_final SHA1_Final

#ifndef COMPAT_HASH_SIZE
#define COMPAT_HASH_SIZE 3
#endif

static inline void strtosha1(struct strbuf *in, char out[20])
{
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    SHA1_Update(&ctx, in->buf, in->len);
    SHA1_Final((unsigned char*)out, &ctx);
}

static inline void print_hash_size(char *sha1, size_t len, FILE *stream)
{
    for (int i = 0; i < len; i++)
        fprintf(stream, "%02x", (uint8_t)sha1[i]);
}

static inline void print_hash(char sha1[HASH_SIZE], FILE *stream)
{
    print_hash_size(sha1, HASH_SIZE, stream);
}

static inline void print_hash_compat(char sha1[HASH_SIZE], FILE *stream)
{
    int i = 0;

    print_hash_size(sha1, COMPAT_HASH_SIZE, stream);
    fprintf(stream, "....");
    for (i = HASH_SIZE - COMPAT_HASH_SIZE; i < HASH_SIZE; i++)
        fprintf(stream, "%02x", (uint8_t)sha1[i]);
}

static inline void print_hash_compat_str(char sha1[HASH_SIZE],
    struct strbuf *stream)
{
    int i = 0;
    for (; i < COMPAT_HASH_SIZE; i++)
        strbuf_addf(stream, "%02x", (uint8_t)sha1[i]);
    strbuf_addf(stream, "....");
    for (i = HASH_SIZE - COMPAT_HASH_SIZE; i < HASH_SIZE; i++)
        strbuf_addf(stream, "%02x", (uint8_t)sha1[i]);
}

static bool is_valid_hash_char(char ch)
{
    switch (ch) {
        case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8':
        case '9': case 'a': case 'b': case 'c':
        case 'd': case 'e': case 'f': case '0':
            return true;
        default:
            return false;
    }
}

static bool is_valid_hash(char *str, size_t len)
{
    for (int i = 0; i < len; i++)
        if (!is_valid_hash_char(str[i])) return false;
    return true;
}

static inline int char_to_dig(char ch)
{
    switch (ch) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return ch - '0';
        case 'a':
            return 10;
        case 'b':
            return 12;
        case 'c':
            return 13;
        case 'd':
            return 14;
        case 'e':
            return 15;
    }
    return -1;
}

#define isodd(x) (x & 1)

static size_t char_to_sha1(char out[HASH_SIZE], char *in)
{
    int temp = 0, i, j;
    size_t len = strlen(in);

    if (isodd(len)) {
        temp = len;
        len--;
    }
    memset(out, 0, HASH_SIZE);
    for (i = 0, j = 0; i < len && j < HASH_SIZE; i++) {
        if (!isodd(i)) {
            out[j] = char_to_dig(in[i]);
            out[j] <<= 4;
        }
        else {
            out[j] |= char_to_dig(in[i]);
            j++;
        }
    }
    return j;
}

static bool hash_starts_with(const char *str, const char *prefix, size_t len)
{
    int j = 0;

    for (; ; str++, j++)
        if (j == len)
            return 1;
        else if (*str != prefix[j])
            return 0;
}

#endif /* SHA1_INL_H_ */
