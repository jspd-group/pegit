#ifndef UTIL_H_
#define UTIL_H_

#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
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

#define strchrnul gitstrchrnul
static inline char *gitstrchrnul(const char *s, int c)
{
    while (*s && *s != c)
        s++;
    return (char *)s;
}

#define REALLOC_ARRAY(x, alloc) (x) = realloc((x), (alloc) * sizeof(*(x)))
#define alloc_nr(x) (((x)+16)*3/2)

#define die(fmt, ...) { printf(fmt); exit(1); }

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
#define error(...)    \
    fprintf(stderr, __VA_ARGS__); \
    exit(1);
#endif

#endif