#ifndef TREE_H_
#define TREE_H_

#include "util.h"
#include "file.h"

#include <dirent.h>
#include <sys/stat.h>

typedef int (*func)(const char *name);

/**
 * list directory details recursively
 */
extern int list_directory_recursive(struct strbuf *complete);

/**
 * list the directory files and folders an don't recurse the sub - folders
 */
extern int list_directory_shallow_sorted(struct strbuf *dir);

/**
 * list the files present in the given directory
 */
extern int list_directory_files(struct strbuf *dir, int count_only);

/**
 * list all folders in the directory `dir`
 */
extern int list_directory_folders(struct strbuf *dir, int count_only);

/**
 * call a specific function for each file in the given directory.
 * This function only acts on the directory given, not on the sub-directories.
 */
extern int for_each_file_in_directory(const char *dir_name, func callback);

/**
 * This function recursively traverses every file in the directories
 * and sub-directories and calls the given callback on every file.
 */
extern int for_each_file_in_directory_recurse(const char *dir_name,
                                              func callback);

#endif
