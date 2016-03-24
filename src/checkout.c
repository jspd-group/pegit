#include "commit.h"
#include "visitor.h"
#include "stage.h"
#include "path.h"

struct revert_opts {
    int dflt;
    int revert_count;
    struct strbuf_list list;
    int force;
    int files;
    char assume;
    int normal;
    int fast;
    int dir;
} rev_opts;

#define MARK_CHECKOUT(flag) (flag |= 0x30)
#define IS_MARKED(flag) (flag & 0x30)

void create_file(struct strbuf *buf, const char *path)
{
    FILE *fp;

    fp = fopen(path, "wb");
    if (!fp) die("unable to modify %s: %s\n", path, strerror(errno));
    fwrite(buf->buf, sizeof(char), buf->len, fp);
    fclose(fp);
}

void revert_file(struct pack_file_cache *cache, struct index *i)
{
    struct strbuf buf = STRBUF_INIT;
    struct strbuf file = STRBUF_INIT;
    struct stat st;
    FILE *fp;

    printf(" rewriting %s\n", i->filename);
    strbuf_add(&buf, cache->cache.buf + i->pack_start, i->pack_len);
    if (stat(i->filename, &st) < 0) {
        if (errno == ENOENT) {
            create_file(&buf, i->filename);
            return;
        } else {
            die(" %s: error: %s\n", i->filename, strerror(errno));
        }
    }

    fp = fopen(i->filename, "wb");
    if (!fp) die("unable to create %s: %s\n", i->filename, strerror(errno));
    fwrite(buf.buf, sizeof(char), buf.len, fp);
    fclose(fp);
    strbuf_release(&buf);
}

void delete_file(struct index *i)
{
    fprintf(stdout, RED" deleting %s\n"RESET, i->filename);
    if (remove(i->filename) < 0) {
        die("%s: %s", i->filename, strerror(errno));
    }
}

void delete_file_str(const char *i)
{
    fprintf(stdout, RED" deleting %s\n"RESET, i);
    if (remove(i) < 0) {
        die("%s: %s", i, strerror(errno));
    }
}

void check_and_mkdir(char *name)
{
    struct stat st;
    char *dir;
    struct strbuf path = STRBUF_INIT;
    size_t len = strlen(name);
    int j = 0;

    if (stat(name, &st) >= 0) {
        return;
    }
    for (int i = 0; i < len; i = j) {
        j++;
        for (; j < len; j++) {
            if (name[j] == '/')
                break;
        }
        if (j == len) break;
        strbuf_add(&path, name + i, j - i);
#if defined(_WIN32)
        if (mkdir(path.buf) < 0) {
#else
        if (mkdir(path.buf, 0777) < 0) {
#endif
            if (errno == EEXIST || !strcmp(path.buf, "."))
                continue;
            else die("can't make %s, %s", path.buf, strerror(errno));
        }
    }
}

void revert_all_files(struct index_list *i)
{
    struct index_list *node = i;
    struct pack_file_cache cache = PACK_FILE_CACHE_INIT;

    cache_pack_file(&cache);
    while (node) {
        check_and_mkdir(node->idx->filename);
        revert_file(&cache, node->idx);
        node = node->next;
    }
    strbuf_release(&cache.cache);
}

struct commit *get_nth_commit(struct commit_list *cl, ssize_t n)
{
    struct commit_list *node = cl;
    if (n > node->count)
        die("You want to go way much back!\n");
    n = node->count - n;
    for (int i = 0; i < n - 1; i++) {
        if (node)
            node = node->next;
        else
            die("Clock was not started at that time!\n");
    }
    return node->item;
}

int revert_to_nth_commit(struct commit_list *cl, ssize_t n)
{
    struct commit *cm = get_nth_commit(cl, n);
    struct index_list *il;

    make_index_list_from_commit(cm, &il);
    revert_all_files(il);
    return 0;
}

int revert_files_commit(struct strbuf_list *list, ssize_t n)
{
    struct index_list *il;
    struct index *i;
    struct commit *cm;
    struct commit_list *cl;
    struct strbuf_list_node *node = list->head->next;
    struct pack_file_cache cache = PACK_FILE_CACHE_INIT;

    make_commit_list(&cl);
    cm = get_nth_commit(cl, n);
    cache_pack_file(&cache);
    make_index_list_from_commit(cm, &il);
    while (node) {
        i = find_file_index_list(il, node->buf.buf);
        if (!i) {
            fprintf(stderr, YELLOW" %s, Not checked in!\n"RESET,
                node->buf.buf);
        } else {
            revert_file(&cache, i);
            MARK_CHECKOUT(i->flags);
        }
        node = node->next;
    }

    return 0;
}

void revert_directory(const char *path, size_t n)
{
    struct visitor v;
    struct strbuf p = STRBUF_INIT;
    struct index *i;
    struct index_list *list;
    struct commit_list *cl;
    struct commit *cm;
    struct pack_file_cache cache = PACK_FILE_CACHE_INIT;

    make_commit_list(&cl);
    cm = get_nth_commit(cl, n);
    cache_pack_file(&cache);
    visitor_init(&v);
    visitor_visit(&v, path);
    make_index_list_from_commit(cm, &list);
    while (visitor_visit_next_entry(&v) == 0) {
        visitor_get_absolute_path(&v, &p);
        i = find_file_index_list(list, p.buf);
        if (!i) {
            fprintf(stderr, YELLOW" %s, Not checked in!\n"RESET, p.buf);
        } else {
            revert_file(&cache, i);
        }
        strbuf_setlen(&p, 0);
    }
}

int revert_parse_options(int argc, char *argv[])
{
    struct strbuf peg_path = STRBUF_INIT;
    struct stat st;

    strbuf_list_init(&rev_opts.list);
    if (argc < 2) {
        fprintf(stdout, "Checking out head\n");
        rev_opts.revert_count = 0;
        rev_opts.dflt = 1;
        return 0;
    }
    for (int i = 1; i < argc; i++) {
        if (!strncmp(argv[i], "~", 1)) {
            rev_opts.revert_count = atoi(argv[i] + 1);
        } else {
            get_peg_path_buf(&peg_path, argv[i]);
            if (stat(peg_path.buf, &st) < 0) {
                if (errno != ENOENT)
                    die("%s: %s\n", argv[i], strerror(errno));
            }
            if (S_ISDIR(st.st_mode)) {
                revert_directory(peg_path.buf, rev_opts.revert_count);
            } else if (S_ISREG(st.st_mode)) {
                strbuf_list_add(&rev_opts.list, peg_path.buf,
                    peg_path.len, 0, 0);
                rev_opts.files = 1;
            }
            rev_opts.normal = 0;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    struct commit_list *cl;
    revert_parse_options(argc, argv);
    make_commit_list(&cl);
    if (!cl) {
        die("You want to go back without any commit!\n");
    }
    if (rev_opts.files) {
        revert_files_commit(&rev_opts.list, rev_opts.revert_count);
    } else if (rev_opts.normal) {
        revert_to_nth_commit(cl, rev_opts.revert_count);
    } else if (rev_opts.dir) {
        return 0;
    }
    return 0;
}
