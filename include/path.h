#ifndef PATH_H_
#define PATH_H_

#include "util.h"
#include "strbuf.h"

enum entry_type { _ERROR_, _DIRECTORY_, _FILE_, _UNKNOWN_ };

extern int is_valid_path(const char *path);
extern void get_peg_path_buf(struct strbuf *bf, const char *path);
extern void get_human_path_buf(struct strbuf *bf, const char *peg_path);

#endif
