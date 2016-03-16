#ifndef SHA1_INL_H_
#define SHA1_INL_H_

#include "util.h"
#include "../sha1/sha1.h"
#include "strbuf.h"

static inline void strtosha1(struct strbuf *in, short out[20])
{
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    SHA1_Update(&ctx, in->buf, in->len);
    SHA1_Final((unsigned char*)out, &ctx);
}


#endif /* SHA1_INL_H_ */