#ifndef UTIL_H_
#define UTIL_H_

#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> /* for strcasecmp() */
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <assert.h>
#include <utime.h>
#include <stdint.h>

#define REALLOC_ARRAY(x, alloc) (x) = realloc((x), (alloc) * sizeof(*(x)))
#define alloc_nr(x) (((x)+16)*3/2)

/* number of bytes required by the hash
 */
#define HASH_SIZE 20

#define die(fmt...) { printf(fmt); exit(1); }

#define DEBUG 1

#ifndef size_t
#define size_t unsigned long long
#endif
#ifndef ssize_t
#define ssize_t long
#endif
#ifndef off_t
#define off_t unsigned long long
#endif

#define comment_line_char '#'
#define ALLOC_GROW(x, nr, alloc) \
    do { \
        if ((nr) > alloc) { \
            if (alloc_nr(alloc) < (nr)) \
                alloc = (nr); \
            else \
                alloc = alloc_nr(alloc); \
            REALLOC_ARRAY(x, alloc); \
        } \
    } while (0)

#ifndef error
#define error(args...) \
    fprintf(stderr, args);
#endif

#define MEMORY_ERROR(prefix, args...) {   \
    fprintf(stderr, "%s: ", prefix);    \
    fprintf(stderr, args);            \
}

/**
 * This global variable stores the name of the program currently being used.
 */
static char program[30];

#define PROGRAM program


#endif