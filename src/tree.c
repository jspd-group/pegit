#include "tree.h"
#include "strbuf.h"

static inline int is_dir(struct stat *st) { return S_ISDIR(st->st_mode); }

static inline int is_file(struct stat *st) { return S_ISREG(st->st_mode); }

static inline int is_peg_directory(struct strbuf *buf)
{
    return !strcmp(buf->buf, PEG_DIR);
}

static inline int is_hidden(struct strbuf *buf)
{
    struct strbuf sb = STRBUF_INIT;
    int hidden = 0;
    extract_filename(&sb, buf->buf);

    /* only unix style hidden files are supported */
    hidden = (buf->buf[0] == '.') ? 1 : 0;
    strbuf_release(&sb);
    return hidden;
}

int list_directory_recursive(struct strbuf *complete)
{
    struct dirent *entry;
    struct stat st;
    int ret;
    DIR *dp = opendir(complete->buf);

    if (!dp) {
        fprintf(stderr, "fatal: %s not a directory\n", complete->buf);
        return -1;
    }

    while ((entry = readdir(dp)) != NULL) {
        strbuf_addch(complete, '/');
        strbuf_addstr(complete, entry->d_name);
        ret = stat(complete->buf, &st);
        if (ret < 0) return ret;

        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
            strbuf_remove(complete, complete->len - strlen(entry->d_name) - 1,
                          strlen(entry->d_name) + 1);
            continue;
        }

        if (is_dir(&st)) {
            ret = list_directory_recursive(complete);
        } else if (is_file(&st)) {
            /* we're only considering files and directories */
            printf("%s\n", complete->buf);
        } else {

            /* Unknown file, still continue */
            fprintf(stderr, "fatal: %s: Unknown file type\n", complete->buf);
            strbuf_remove(complete, complete->len - strlen(entry->d_name) - 1,
                          strlen(entry->d_name) + 1);
            return 0;
        }
        strbuf_remove(complete, complete->len - strlen(entry->d_name) - 1,
                      strlen(entry->d_name) + 1);
    }
    (void)closedir(dp);
    return 0;
}

int list_directory_shallow_sorted(struct strbuf *dir)
{
    struct dirent *entry;
    DIR *dp;
    struct stat st;
    int ret = 0;

    dp = opendir(dir->buf);

    if (!dp) {
        fprintf(stderr, "fatal: %s: not a directory\n", dir->buf);
        return -1;
    }

    strbuf_addch(dir, '/');

    while ((entry = readdir(dp)) != NULL) {
        strbuf_addstr(dir, entry->d_name);
        ret = stat(dir->buf, &st);
        if (ret < 0) die("fatal: %s: can't do stat\n", entry->d_name);
        if (is_dir(&st)) {
            fprintf(stdout, "%s/\n", entry->d_name);
        } else if (is_file(&st)) {
            fprintf(stdout, "%s\n", entry->d_name);
        } else {
            fprintf(stdout, "%s : Unknown file exists in the project\n",
                    entry->d_name);
        }
        strbuf_setlen(dir, dir->len - strlen(entry->d_name));
    }
    (void)closedir(dp);
    return ret;
}

int list_directory_folders(struct strbuf *dir, int count_only)
{
    struct dirent *entry;
    DIR *dp;
    struct stat st;
    int ret = 0, count;

    dp = opendir(dir->buf);

    if (!dp) {
        fprintf(stderr, "fatal: %s: not a directory\n", dir->buf);
        return -1;
    }

    strbuf_addch(dir, '/');

    while ((entry = readdir(dp)) != NULL) {
        strbuf_addstr(dir, entry->d_name);
        ret = stat(dir->buf, &st);
        if (ret < 0) die("fatal: %s: can't do stat\n", entry->d_name);
        if (is_dir(&st)) {
            if (!count_only) fprintf(stdout, "%s/\n", entry->d_name);
            count++;
        }
        strbuf_setlen(dir, dir->len - strlen(entry->d_name));
    }
    (void)closedir(dp);
    return count;
}

int list_directory_files(struct strbuf *dir, int count_only)
{
    struct dirent *entry;
    DIR *dp;
    struct stat st;
    int ret = 0, count = 0;

    dp = opendir(dir->buf);
    if (!dp) {
        fprintf(stderr, "fatal: %s: not a directory\n", dir->buf);
        return -1;
    }
    strbuf_addch(dir, '/');

    while ((entry = readdir(dp)) != NULL) {
        strbuf_addstr(dir, entry->d_name);
        ret = stat(dir->buf, &st);
        if (ret < 0) die("fatal: %s: can't do stat\n", entry->d_name);
        if (is_file(&st)) {
            if (!count_only) fprintf(stdout, "%s\n", entry->d_name);
            count++;
        }
        strbuf_setlen(dir, dir->len - strlen(entry->d_name));
    }

    (void)closedir(dp);
    return count;
}

int for_each_file_in_directory(const char *dir_name, func callback)
{
    struct dirent *entry;
    DIR *dp;
    struct stat st;
    int ret = 0, count = 0;
    struct strbuf dir = STRBUF_INIT;

    strbuf_addstr(&dir, dir_name);
    dp = opendir(dir.buf);
    if (!dp) {
        fprintf(stderr, "fatal: %s: not a directory\n", dir.buf);
        return -1;
    }
    strbuf_addch(&dir, '/');

    while ((entry = readdir(dp)) != NULL) {
        strbuf_addstr(&dir, entry->d_name);
        ret = stat(dir.buf, &st);
        if (ret < 0) die("fatal: %s: can't do stat\n", entry->d_name);
        if (is_file(&st)) {
            ret = callback(dir.buf);
            if (!ret) count++;
        }
        strbuf_setlen(&dir, dir.len - strlen(entry->d_name));
    }

    strbuf_release(&dir);
    (void)closedir(dp);
    return count;
}

int for_each_file_in_directory_recurse(const char *dir_name, func callback)
{
    struct dirent *entry;
    struct stat st;
    int ret, count = 0, err_threshhold = 0;
    struct strbuf complete = STRBUF_INIT;
    strbuf_addstr(&complete, dir_name);

    DIR *dp = opendir(complete.buf);
    if (!dp) {
        fprintf(stderr, "fatal: %s not a directory\n", complete.buf);
        return -1;
    }

    while ((entry = readdir(dp)) != NULL) {
        strbuf_addch(&complete, '/');
        strbuf_addstr(&complete, entry->d_name);
        ret = stat(complete.buf, &st);
        if (ret < 0) {
            die("fatal: %s: can't do stat.\n", complete.buf);
        }

        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..") ||
            !strcmp(entry->d_name, PEG_DIR)) {
            strbuf_setlen(&complete, complete.len - strlen(entry->d_name) - 1);
            continue;
        }

        if (is_dir(&st)) {
            ret = for_each_file_in_directory_recurse(complete.buf, callback);
            count += ret;
        } else if (is_file(&st)) {
            ret = callback(complete.buf);
            if (!ret) count++;
        } else {

            /* Unknown file, still continue */
            err_threshhold++;
            fprintf(stderr, "fatal: %s: Is this a file?\n", complete.buf);
            if (err_threshhold > 10) {
                fprintf(stderr, "fatal: So many unknown files exist\n");
                return -1;
            }
        }
        strbuf_setlen(&complete, complete.len - strlen(entry->d_name) - 1);
    }

    strbuf_release(&complete);
    (void)closedir(dp);
    return count;
}
