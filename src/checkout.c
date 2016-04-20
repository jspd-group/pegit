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

int remove_file(const char *path)
{
    if (remove(path) < 0) {
        die("%s: %s, can't delete.\n", path, strerror(errno));
    }
    return 0;
}

int remove_directory(const char *path)
{
    struct dirent *entry;
    DIR *dp;
    struct stat st;
    int ret = 0, count = 0;
    struct strbuf dir = STRBUF_INIT;

    strbuf_addstr(&dir, path);
    dp = opendir(dir.buf);
    if (!dp) {
        die("%s: not a directory, %s\n", dir.buf, strerror(errno));
    }
    strbuf_addch(&dir, '/');

    while ((entry = readdir(dp)) != NULL) {
        strbuf_addstr(&dir, entry->d_name);

        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..") ||
            !strcmp(entry->d_name, PEG_DIR)) {
            strbuf_setlen(&dir, dir.len - strlen(entry->d_name));
            continue;
        }

        ret = stat(dir.buf, &st);
        if (ret < 0) die("%s: can't do stat, %s\n", entry->d_name, strerror(errno));
        if (S_ISDIR(st.st_mode)) {
            remove_directory(dir.buf);
            rmdir(dir.buf);
            if (ret) {
                die("%s: can't delete, %s", dir.buf, strerror(errno));
            }
        } else if (S_ISREG(st.st_mode)) {
            remove_file(dir.buf);
        }
        strbuf_setlen(&dir, dir.len - strlen(entry->d_name));
    }

    strbuf_release(&dir);
    (void)closedir(dp);
    return count;
}

void clean_directory(const char *path)
{
    struct node *node = root;
    char ans[10];

    if (root && (count_modified || count_new)) {
        printf("Uncommitted changes exist in project: \n");
        while (node) {
            printf("%s %s\n", node->status == 1 ? "M" : "N",
                                node->name);
            node = node->next;
        }
        printf("Do you want to continue?\n");
        gets(ans);
        if (!strncmp(ans, "y", 1) || !strcmp(ans, "Y"))
            ;
        else {
            printf("aborting...\n");
            exit(0);
        }
    }
    for_each_file_in_directory_recurse(path, remove_file);
    remove_directory(path);
}

void create_file(struct strbuf *buf, const char *path)
{
    FILE *fp;

    fp = fopen(path, "wb");
    if (!fp) fatal("unable to create %s: %s\n", path, strerror(errno));
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
            fatal(" %s: error: %s\n", i->filename, strerror(errno));
        }
    }

    fp = fopen(i->filename, "wb");
    if (!fp) fatal("unable to modify %s: %s\n", i->filename, strerror(errno));
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

void revert_files_hard()
{
    struct commit_list *cl;
    struct index_list *il;
    struct index_list *node;
    struct pack_file_cache cache = PACK_FILE_CACHE_INIT;

    clean_directory(".");
    make_commit_list(&cl);
    il = get_head_commit_list(cl);
    revert_all_files(il);
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

void revert_directory(struct pack_file_cache *cache, struct index_list *list,
    const char *path, size_t n)
{
    struct visitor v;
    struct strbuf p = STRBUF_INIT;
    struct index *i;

    visitor_init(&v);
    visitor_visit(&v, path);

    while (visitor_visit_next_entry(&v) == 0) {
        visitor_get_absolute_path(&v, &p);
        if (!strcmp(v.current_entry->d_name, ".peg")) {
            strbuf_setlen(&p, 0);
            continue;
        }
        if (v.entry_type == _FILE) {
            i = find_file_index_list(list, p.buf);
            if (!i) {
                printf(YELLOW" %s"RESET": Not Checked in!\n", p.buf);
            } else
                revert_file(cache, i);
        } else if (v.entry_type == _FOLDER) {
            revert_directory(cache, list, p.buf, n);
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
        } else if (!strcmp(argv[i], "--hard") || !strcmp(argv[i], "-h")) {
            revert_files_hard();
            return 0;
        } else {
            get_peg_path_buf(&peg_path, argv[i]);
            if (stat(peg_path.buf, &st) < 0) {
                if (errno != ENOENT)
                    die("%s: %s\n", argv[i], strerror(errno));
            }
            if (S_ISDIR(st.st_mode)) {
                struct commit_list *cl;
                struct commit *cm;
                struct pack_file_cache cache = PACK_FILE_CACHE_INIT;
                struct index_list *list;

                make_commit_list(&cl);
                cm = get_nth_commit(cl, rev_opts.revert_count);
                cache_pack_file(&cache);
                make_index_list_from_commit(cm, &list);
                revert_directory(&cache, list, peg_path.buf, rev_opts.revert_count);
                invalidate_cache(&cache);
                commit_list_del(&cl);
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

int checkout(int argc, char *argv[])
{
    struct commit_list *cl;
    printf("checking status...\n");
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
