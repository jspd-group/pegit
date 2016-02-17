#ifndef MZ_WRAPPER_H_
#define MZ_WRAPPER_H_

#include "../miniz/miniz.h"
#include "strbuf.h"

#ifdef compress
#undef compress
#endif

#ifdef uncompress
#undef uncompress
#endif

/*
 * this function compresses the src to dest to a level
 * and returns 0 if successfully compressed otherwise returns -1
 * use level 0 for no compression, 9 for best compression
 */
extern int compress(struct strbuf *src, struct strbuf *dest, int level);

/*
 * compress the src to dest with Z_DEFAULT_COMPRESSION level
 */
extern int compress_default(struct strbuf *src, struct strbuf *dest);

/*
 * uncompresses a string buffer from the src and put the result
 * in dest buffer. Returns 0 if successful otherwise -1
 */
extern int uncompress(struct strbuf *src, struct strbuf *dest);


#endif /* MZ_WRAPPER_H_ */
