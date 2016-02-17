#ifndef SHA1_INL_H_
#define SHA1_INL_H_

#include "util.h"
#include "sha1.h"
#include "strbuf.h"

#include <string.h>

void sha1(struct strbuf *in, char out[20]) {
    sha1nfo ctx;
    sha1_init(&ctx);
    sha1_write(&ctx, in->buf, in->len);

    strncpy(sha1_result(&ctx), out, 20);
}


#endif /* SHA1_INL_H_ */