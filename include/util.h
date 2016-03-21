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
    do {                                                                       \
        printf(__VA_ARGS__);                                                   \
        exit(1);                                                               \
    } while(0);

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

#ifndef HINT
#define HINT 2048
#endif

struct strbuf;

extern int __compress__(struct strbuf *src, struct strbuf *dest, int level);
extern int compress_default(struct strbuf *src, struct strbuf *dest);
extern int decompress(struct strbuf *src, struct strbuf *dest);

#define PROGRAM             program
#define PEG_DIR             ".peg"
#define PEG_NAME            "peg"
#define CACHE_DIR           "cache"
#define DB_DIR              "db"
#define COMMIT_DIR          "commit"
#define DESCRIPTION_FILE    ".peg/desc"
#define CACHE_INDEX_FILE    ".peg/cache/cache.idx"
#define CACHE_PACK_FILE     ".peg/cache/cache.pack"
#define FILE_INDEX_FILE ".peg/db/db.idx"
#define COMMIT_INDEX_FILE ".peg/commit/commit.idx"
#define PACK_FILE ".peg/db/db.pack"
#define HEAD_FILE ".peg/HEAD"
#define HEAD_POINTER_NAME "HEAD"

/**
 * basic colors used for output
 */
#define BLACK "\x1b[30m"
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define WHITE "\x1b[37m"
#define RESET "\x1b[0;0m"

/**
 * text attributes
 */
#define BOLD "\x1b[1m"

#endif
