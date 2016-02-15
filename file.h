#ifndef FILE_H
#define FILE_H

#include "util.h"
#include "strbuf.h"

/**
 *  Various file structures useful for file handling
 * ----------------------------------------------------------------------------
 */

/**
 * (should not be used without file_spec)
 */
struct file_internal {
    void *data;    /* data inside the file though not always text. But can
                             be binary data also */
    size_t size;    /* size of the file */
};

/**
 * struct filespec:
 *   used to store all the file information. So to be independant of
 * slow system calls. Use ordinary FILE pointer if you don't need such
 * details. Only file reading was sufficient
 */
struct filespec {
    struct strbuf fname;    /* name of the file */
    time_t last_modified;    /* time of the last modified */
    struct stat st;    /* stat of the file for additional information */
    struct file_internal internal;
    char sha1[HASH_SIZE]; /* hash generated of the file */

    /**
     * (don't use in case you don't need the information of the directory)
     * Name of the directory in which this file is located.
     * Name of the directory is relative to the main project directory.
     */
    struct strbuf dir_name;
};


/**
 * store the whole file in the strbuf array. Each element in the array
 * corresponds to a particular line in the file. Using such format is
 * useful for making the delta faster (a little). It may take more memory.
 */
struct delta_file {
    struct strbuf *arr;    /* main array */
    size_t size;        /* size of the array */
    char delim;        /* delimiter on which we are dividing the lines */
};

/**
 * returns the number of characters in the file
 * TODO: find a better way to find the file length
 */
static inline size_t file_length(FILE *file) {
    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    return length;
}

/**
 * Routines related to above structures
 * ----------------------------------------------------------------------------
 */

/**
 * struct file_internal
 * ----------------------------------------------------------------------------
 */

/**
 * Make a file_internal from a given file descritpor. Returns 0 if successful,
 * -1 if failed or some error occurred. size is the number of characters to
 * read from the file. Memory will be allocated to the out->data.
 */
extern int file_internal_init_fd(struct file_internal *out,
                                 int fd, size_t size);


/**
 * Make a file_internal from a given FILE pointer. Returns 0 if reading was
 * successful from the given FILE pointer, otherwise return -1 on error.
 */
extern int file_internal_init_file(struct file_internal *out,
                                   FILE *f, size_t size);

/**
 * struct file_spec
 * ----------------------------------------------------------------------------
 */

/**
 * Initialise a file_spec structure from a given file_name or its path.
 * returns 0 if successful otherwise -1 on error.
 */
extern int filespec_init(struct filespec *fs, const char *name);

#endif /* FILE_H */