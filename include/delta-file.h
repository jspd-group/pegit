#ifndef DELTA_FILE_H_
#define DELTA_FILE_H_

#include "file.h"
#include "strbuf.h"
#include "util.h"

/**
 * store the whole file in the strbuf array. Each element in the array
 * corresponds to a particular line in the file. Using such format is
 * useful for making the delta faster (a little). It may take more memory.
 */
struct deltafile {
    struct strbuf file; /* file contents */
    ssize_t *arr;       /* main array */
    size_t size;        /* size of the array */
    char delim;         /* delimiter on which we are dividing the lines */
};

/**
 * deltafile routines
 * -------------------------------
 */

/**
 * Initialise a deltafile by specifying its size
 */
extern int deltafile_init(struct deltafile *df, size_t size, char delim);

/**
 * Initialise  deltafile from a given filespec
 */
extern int deltafile_init_filespec(struct deltafile *df, struct filespec *fs,
                                   char delim);

/**
 * Initialise the deltafile from a strbuf
 */
extern int deltafile_init_strbuf(struct deltafile *df, struct strbuf *buf,
                                 char delim);

/**
 * free the memory
 */
extern void deltafile_free(struct deltafile *df);

extern size_t count_lines(struct strbuf *buf);
#endif // DELTA_FILE_H_
