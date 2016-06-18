#include "path.h"

int is_valid_path(const char *path)
{
    struct stat st;
    if (stat(path, &st) < 0) {
        return _ERROR_;
    }
    if (S_ISDIR(st.st_mode))
        return _DIRECTORY_;
    else if (S_ISREG(st.st_mode))
        return _FILE_;
    return _UNKNOWN_;
}

void get_peg_path_buf(struct strbuf *bf, const char *path)
{
    if (path[0] != '.' && path[1] != '/') {
        strbuf_addstr(bf, "./");
    }
    strbuf_addstr(bf, path);
}

void get_human_path_buf(struct strbuf *bf, const char *peg_path)
{
    if (peg_path[0] == '.' && peg_path[1] == '/')
        strbuf_add(bf, peg_path + 2, strlen(peg_path));
    else
        strbuf_add(bf, peg_path, strlen(peg_path));
}
