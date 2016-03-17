#include "stage.h"
#include "sha1-inl.h"
#include "tree.h"
#include "file.h"

char stage_usage[] =
PEG_NAME " stage : add project files to the stage area\n"
"    Usage:    " PEG_NAME " stage [OPTIONS] files...\n"
"        where OPTIONS are:\n"
"        -a, --all         add all the files that have been modified.\n"
"        -i, --ignore=     ignore the given files and add all other.\n"
"         .                similar to -a\n"
"\n";

int read_file_from_database(const char *path, struct strbuf *buf)
{
    find_file_from_head_commit(path, buf);
    if (buf->len) return 0;
    return 1;
}

struct file_list *last, *head;
struct stage_options opts;

void init_stage()
{
    last = NULL;
    head = NULL;
    opts.all = 0;
    opts.ignore = 0;
    opts.ignarr = NULL;
    opts.add = NULL;

}

void file_list_init(struct file_list *fl)
{
    strbuf_init(&fl->path, 0);
    strbuf_init(&fl->file, 0);
    fl->next = NULL;
    fl->old = 0;
}

void file_list_add(struct file_list *node)
{
    if (!head) {
        head = node;
        last = node;
        return;
    }
    last->next = node;
    last = node;
}

#define STAGE_DATA_INIT { 0, 0 }

void print_stage_stats(struct stage_stats *data)
{
    if (data->files_modified || !data->new_files)
        fprintf(stdout, "%llu %s modified, ", data->files_modified,
            data->files_modified <= 1 ? "file" : "files");
    if (data->new_files || !data->files_modified) {
        fprintf(stdout, "%llu new %s added. ", data->new_files,
            data->new_files <= 1 ? "file" : "files");
    }
    fprintf(stdout, "\n");
}

void load_old_cache_file(struct cache_object *co)
{
    cache_object_init(co);
}

bool is_marked_as_ignored(const char *name)
{
    struct strbuf temp = STRBUF_INIT;
    for (int i = 0; i < opts.ignore; i++) {
        strbuf_addstr(&temp, "./");
        strbuf_addbuf(&temp, &opts.ignarr[i]);
        if (!strncmp(name, opts.ignarr[i].buf, opts.ignarr[i].len - 1)
            || !strncmp(name, temp.buf, temp.len - 1)) {
            strbuf_release(&temp);
            return true;
        }
        strbuf_release(&temp);
        strbuf_init(&temp, 0);
    }
    return false;
}

int is_modified(const char *name)
{
    struct filespec fs;
    struct strbuf a = STRBUF_INIT, b = STRBUF_INIT;
    struct file_list *node;

    if (is_marked_as_ignored(name)) return 1;
    filespec_init(&fs, name, "r");
    filespec_read_safe(&fs, &a);
    if (read_file_from_database(name, &b)) {
        stats.new_files++;
        node = MALLOC(struct file_list, 1);
        if (!node)
            die("fatal: no memory available\n\t:(\n");
        file_list_init(node);
        node->old = false;
        strbuf_init(&node->path, 0);
        strbuf_addstr(&node->path, name);
        /* defer hash computation to later time */
        node->file.buf = a.buf;
        node->file.len = a.len;
        node->file.alloc = a.alloc;
        node->next = NULL;
        file_list_add(node);
        return 0;
    }
    if (strbuf_cmp(&a, &b)) {
        stats.files_modified++;
        node = MALLOC(struct file_list, 1);
        if (!node)
            die("fatal: no memory available\n\t:(\n");
        node->old = true;
        strbuf_init(&node->path, 0);
        strbuf_addstr(&node->path, name);
        strbuf_release(&b);
        node->file.buf = a.buf;
        node->file.len = a.len;
        node->file.alloc = a.alloc;
        node->next = NULL;
        file_list_add(node);
        return 0;
    }
    return 1;
}

void process_node(struct cache_object *cache, struct file_list *node)
{
    struct cache_index_entry_list *n = MALLOC(struct cache_index_entry_list, 1);
    char sha1[HASH_SIZE];

    n->len = node->file.len;
    strbuf_init(&n->file_path, 0);
    strbuf_addbuf(&n->file_path, &node->path);
    strtosha1(&node->file, sha1);
    strcpy((char *)node->sha1, (const char *)sha1);
    n->next = NULL;
    cache_object_addindex(cache, &node->file, n);
}

void print_file_list_node(struct file_list *node)
{
    printf("%s\t%s\n", node->old ? "M" : "N", node->path.buf);
}

void cache_files()
{
    struct cache_object cache;
    struct file_list *node = head;

    cache_object_init(&cache);
    while (node) {
        if (!find_file_from_cache(node->path.buf, &cache)) {
            print_file_list_node(node);
            process_node(&cache, node);
        }
        else {
            printf("ignored: %s: already staged\n", node->path.buf);
        }
        node = node->next;
    }
    cache_object_write(&cache);

    /* a little memory management is required here */
    return;
}

int detect_and_add_files()
{
    head = NULL;
    last = NULL;
    size_t count;

    count = for_each_file_in_directory_recurse(".", is_modified);
    stats.total = count;
    cache_files();
    return 0;
}

void print_stage_usage()
{
    fprintf(stdout, "%s", stage_usage);
}

size_t findch(char *a, char delim)
{
    char *temp = a;
    while (*a++ != '\0') if (*a == delim) return a - temp;
    return -1;
}

void parse_ignore_list(char *argv)
{
    struct strbuf buf = STRBUF_INIT;
    strbuf_addstr(&buf, argv + findch(argv, '=') + 1);

    size_t size = strbuf_count(&buf, ';') + 1;
    size_t i = 0;
    opts.ignarr = MALLOC(struct strbuf, size);
    opts.ignore = size;
    char *tok = strtok(buf.buf, ";");

    while (tok != NULL) {
        strbuf_init(opts.ignarr + i, 128);
        strbuf_addstr(opts.ignarr + i, tok);
        tok = strtok(NULL, ";");
        i++;
    }
}

void process_argument(int i, char *argv)
{
    if (i == 1 && !strcmp(argv, ".")) {
        opts.all = 1;

    }
    else if (!strcmp(argv, "-h") || !strcmp(argv, "--help")) {
        print_stage_usage();
        exit(0);
    }
    else if (!strcmp(argv, "-a") || !strcmp(argv, "--all")) {
        opts.all = 1;
    }
    else if (!strncmp(argv, "-i", 2) || !strncmp(argv, "--ignore", 8)) {
        parse_ignore_list(argv);
    }
    else if (!strcmp(argv, "reset")) {
        struct cache_object co;
        cache_object_init(&co);
        cache_object_clean(&co);
        exit(0);
    }
    else if (!opts.all) {
        if (!is_modified(argv))
            stats.total++;
    }
}

int parse_arguments(int argc, char *argv[])
{
    if (argc < 2) {
        die("fatal: atleast one argument is required.\n\t:(\n");
    }
    for (int i = 1; i < argc; i++) {
        process_argument(i, argv[i]);
    }
    return 0;
}

int stage_main(int argc, char *argv[])
{
    init_stage();
    parse_arguments(argc, argv);
    if (opts.all) {
        detect_and_add_files();
    }
    else {
        cache_files();
    }
    print_stage_stats(&stats);
    return 0;
}
