#ifndef SHA1_INL_H_
#define SHA1_INL_H_

#include "util.h"
#include "../sha1/sha1.h"
#include "strbuf.h"

#include <string.h>

void mystrcpy(char *dest, const char *src, size_t size) {
    for (int i = 0; i < size; i++)
        dest[i] = src[i];
}

void strtosha1(struct strbuf *in, char out[20])
{
    sha1nfo ctx;
    sha1_init(&ctx);
    sha1_write(&ctx, in->buf, in->len);
    char *res = sha1_result(&ctx);
#if DEBUG
    for (int i = 0; i < HASH_SIZE; i++)
        printf("%c", res[i]);
#endif

    mystrcpy(out, res, HASH_SIZE);
}


#endif /* SHA1_INL_H_ */