#ifndef UTIL_H_
#define UTIL_H_

#define MSVC _MSC_VER

#ifndef MSVC
#include <unistd.h>
#endif
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

#define REALLOC_ARRAY(x, alloc) (x) = realloc((x), (alloc) * sizeof(*(x)))
#define alloc_nr(x) (((x) + 16) * 3 / 2)

/* number of bytes required by the hash
 */
#define HASH_SIZE 20

#define die(...)                                                               \
    {                                                                          \
        printf(__VA_ARGS__);                                                   \
        exit(1);                                                               \
    }

#define DEBUG 0

#define comment_line_char '#'
#define ALLOC_GROW(x, nr, alloc)                                               \
    do {                                                                       \
        if ((nr) > alloc) {                                                    \
            if (alloc_nr(alloc) < (nr))                                        \
                alloc = (nr);                                                  \
            else                                                               \
                alloc = alloc_nr(alloc);                                       \
            REALLOC_ARRAY(x, alloc);                                           \
        }                                                                      \
    } while (0)

#define MALLOC(type, size) (type *) malloc(sizeof(type) * size)

#ifndef error
#define error(...) fprintf(stderr, __VA_ARGS__);
#endif

#define MEMORY_ERROR(prefix, ...)                                              \
    {                                                                          \
        fprintf(stderr, "%s: ", prefix);                                       \
        fprintf(stderr, __VA_ARGS__);                                          \
    \
}

#ifndef isspace
#define isspace(x) ((x) == ' ')
#endif

#ifndef tolower
#define tolower(x) ((x) - 'A' + 'a')
#endif
/**
 * This global variable stores the name of the program currently being used.
 */
// static char program[30];

#ifdef compress
#undef compress
#endif

#ifdef uncompress
#undef uncompress
#endif

struct strbuf;

extern int __compress__(struct strbuf *src, struct strbuf *dest, int level);
extern int compress_default(struct strbuf *src, struct strbuf *dest);
extern int decompress(struct strbuf *src, struct strbuf *dest);

#define PROGRAM program
#define PEG_DIR ".peg"

#endif