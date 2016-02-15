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
struct filecache {
    struct strbuf *data;    /* data inside the file though not always text. But can
                             be binary data also */
};

/**
 * struct filespec:
 *   used to store all the file information. So to be independant of
 * slow system calls. Use ordinary FILE pointer if you don't need such
 * details. This struct also do the file caching for faster reading.
 */
struct filespec {
    struct strbuf fname;    /* name of the file */
    FILE *file;
    time_t last_modified;    /* time of the last modified */
    struct stat st;    /* stat of the file for additional information */
    struct filecache cache;
    char sha1[HASH_SIZE]; /* hash generated of the file */
    size_t length;      /* length of the file */

    /**
     * (don't use in case you don't need the information of the directory)
     * Name of the directory in which this file is located.
     * Name of the directory is relative to the main project directory.
     */
    struct strbuf dir_name;

    int cached; /* true if the whole file is cached */
};

/**
 * returns the number of characters in the file
 * TODO: find a better way to find the file length
 */
extern size_t file_length(FILE *file);

/**
 * Routines related to above structures
 * ----------------------------------------------------------------------------
 */

/**
 * struct filecache
 * ----------------------------------------------------------------------------
 */

/**
 * Make a file_internal from a given file descritpor. Returns 0 if successful,
 * -1 if failed or some error occurred. size is the number of characters to
 * read from the file. Memory will be allocated to the out->data.
 */
extern int filecache_init_fd(struct file_internal *out,
                                 int fd, size_t size);


/**
 * Make a file_internal from a given FILE pointer. Returns 0 if reading was
 * successful from the given FILE pointer, otherwise return -1 on error.
 */
extern int filecache_init_file(struct file_internal *out,
                                   FILE *f, size_t size);

/**
 * struct filespec
 * ----------------------------------------------------------------------------
 */

/**
 * Initialise a file_spec structure from a given file_name or its path.
 * returns 0 if successful otherwise -1 on error.
 */
extern int filespec_init(struct filespec *fs, const char *name);


/**
 * release the resources allocated to the fs
 */
extern void filespec_free(struct filespec *fs);

/**
 * Calculate 20 byte SHA1 hash from a given filespec structure.
 * 
 */
extern int filespec_sha1(struct filespec *fs, char sha1[20]);

/**
 * Read the file using a safe way. May be slow.
 */
extern int filespec_read_safe(struct filespec *fs, struct strbuf *buf);

/**
 * An UNSAFE way to read the file. If specified using fast flag, then
 * reading may be done by just copying the pointer to cached file.
 */
extern int filespec_read_unsafe(struct filespec *fs, struct strbuf *buf,
                                                                    int fast);

/**
 * cache the file to the main memory to speed up the reading
 */
extern int filespec_cachefile(struct filespec *fs);

/**
 * remove the cache of the file to release some memory
 */
void filespec_remove_cache(struct filespec *fs);

#endif /* FILE_H */