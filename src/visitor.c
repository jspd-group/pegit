#include "visitor.h"

void visitor_init(struct visitor *v)
{
    strbuf_init(&v->path, 1024);
    v->root = NULL;
    v->visited_all = 0;
    v->current_entry = NULL;
}

void visitor_close(struct visitor *v)
{
    if (v->root) closedir(v->root);
    strbuf_release(&v->path);
    v->root = NULL;
}

int visitor_visit(struct visitor *v, const char *path)
{
    v->root = opendir(path);
    if (!v->root) {
        fatal("%s: Not a directory.\n", path);
        return -1;
    }
    strbuf_addstr(&v->path, path);
    v->entry_type = _FOLDER;
    v->current_entry = NULL;
    return 0;
}

int visitor_visit_next_entry(struct visitor *v)
{
    struct stat st;
    if (v->visited_all) return 1;

    while ((v->current_entry = readdir(v->root)) != NULL) {
        if (!strcmp(v->current_entry->d_name, ".") ||
            !strcmp(v->current_entry->d_name, ".."))
            continue;
        else {
            strbuf_addch(&v->path, '/');
            strbuf_addstr(&v->path, v->current_entry->d_name);
            if (stat(v->path.buf, &st) < 0) {
                fatal("%s: can't do stat, %s\n", v->path.buf,
                    strerror(errno));
                strbuf_setlen(&v->path,
                    v->path.len - strlen(v->current_entry->d_name) - 1);
                return -1;
            }

            if (S_ISDIR(st.st_mode))
                v->entry_type = _FOLDER;
            else if (S_ISREG(st.st_mode))
                v->entry_type = _FILE;
            else
                v->entry_type = _UNKNOWN;

            strbuf_setlen(&v->path,
                          v->path.len - strlen(v->current_entry->d_name) - 1);

            /* currently there may be remaining entries */
            return 0;
        }
    }

    v->visited_all = 1;
    /* all the entries have been visited */
    return 1;
}

int visitor_visit_child_directory(struct visitor *v)
{
    if (v->current_entry == NULL) {
        die("BUG: v->current_entry uninitialised.\n");
        return -1;
    }

    if (v->entry_type != _FOLDER) {
        die("%s: %s: not a directory\n", v->path.buf,
                v->current_entry->d_name);
        return -1;
    }
    strbuf_addch(&v->path, '/');
    strbuf_addstr(&v->path, v->current_entry->d_name);
    return 0;
}

int visitor_visit_parent(struct visitor *v)
{
    if (!v->root) {
        die("BUG: v->root uninitialised\n");
        return -1;
    }
    if ((v->path.len == 0) || v->path.buf[v->path.len - 1] == '.') {
        strbuf_addch(&v->path, '/');
        strbuf_add(&v->path, "..", 2);
        return 0;
    }

    while ((v->path.len) && v->path.buf[v->path.len--] != '/') /* do nothing */;
    v->path.buf[v->path.len] = '\0';
    return 0;
}

int visitor_make_folder(struct visitor *v, const char *name)
{
    struct strbuf absolute_path = STRBUF_INIT;

    strbuf_addbuf(&absolute_path, &v->path);
    strbuf_addch(&absolute_path, '/');
    strbuf_addstr(&absolute_path, name);
#if defined(_WIN32)
    if (mkdir(absolute_path.buf) < 0) {
#else
    if (mkdir(absolute_path.buf, 0777) < 0) {
#endif
        die("%s: can't make a folder, %s.\n",
                absolute_path.buf, strerror(errno));
        return -1;
    }
    return 0;
}

int visitor_check_or_mkdir(struct visitor *v, const char *path)
{
    struct stat st;

    if (stat(path, &st) < 0) {
        if (errno == ENOENT) {
            return visitor_make_folder(v, path);
        } else {
            fatal("unable to revert error occured, %s\n", strerror(errno));
        }
        return -1;
    }
    return 0;
}
