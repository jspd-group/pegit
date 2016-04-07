#include "stage.h"
#include "sha1-inl.h"
#include "tree.h"
#include "file.h"
#include "path.h"

struct stage_options {
    int all;
    int ignore;
    int all_dot;
    int verbose;
    int more_output;
    struct strbuf *ignarr;
    struct strbuf *add;
    struct index_list *head;
    struct pack_file_cache cache;
};

char stage_usage[] = YELLOW PEG_NAME
    " insert"RESET" : add project files to the stage area\n\n"
    "    Usage:    "YELLOW PEG_NAME " insert [options] <files...>" RESET "\n\n"
    "        options include:\n"
    "        "YELLOW"-a"RESET", "YELLOW"--all\n"RESET
    "                add all the files that have been modified,\n"
    "        "YELLOW"-i"RESET", "YELLOW"--ignore=path1;path2..."RESET"   \n"
    "                ignore the given files and add all other,\n"
    "        "YELLOW"-v"RESET", "YELLOW"--verbose"RESET"\n"
    "                print the name of the added files, similar to -a.\n"
    "        "YELLOW"-m"RESET", "YELLOW"--more"RESET"\n"
    "                display more output.\n"
    "\n\n"
    "Example usage: \n\t'"YELLOW PEG_NAME " insert -a " RESET
    "'(or '"YELLOW PEG_NAME " insert ." RESET "')\n"
    "        will add all the files in the current directory\n\n"
    "\t'"YELLOW PEG_NAME " insert -i=docs/file.txt docs" RESET "'\n"
    "        will add all the files in the docs directory except file.txt."
    "\n";

struct stage_stats {
    size_t files_modified;
    size_t new_files;
    size_t ignored;
    size_t total;
} stats = {0, 0, 0, 0};


struct file_list *last, *head;
struct stage_options opts = {
    .cache = PACK_FILE_CACHE_INIT
};

int read_file_from_database(const char *path, struct strbuf *buf)
{
    struct index *idx;

    if (!opts.head)
        return 0;

    idx = find_file_index_list(opts.head, path);
    if (!idx) return false;
    strbuf_add(buf, opts.cache.cache.buf + idx->pack_start, idx->pack_len);
    return 1;
}

void init_stage()
{
    last = NULL;
    head = NULL;
    opts.all = 0;
    opts.ignore = 0;
    opts.verbose = 0;
    opts.ignarr = NULL;
    opts.add = NULL;
    opts.more_output = 0;

    struct commit_list *cl;
    make_commit_list(&cl);
    opts.head = get_head_commit_list(cl);
    cache_pack_file(&opts.cache);
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

#define STAGE_DATA_INIT                                                        \
    {                                                                          \
        0, 0                                                                   \
    }

void print_stage_stats(struct stage_stats *data)
{
    size_t modified = 0, added = 0, ignored = data->ignored;



#ifdef _WIN32
    if (data->files_modified && (((int)(data->files_modified - data->ignored)) > 0)) {
        fprintf(stdout, " %llu %s modified\n", data->files_modified,
                data->files_modified <= 1 ? "file" : "files");
    }
#else
    if (data->files_modified && (data->files_modified - data->ignored)) {
        fprintf(stdout, " %zu %s modified\n", data->files_modified,
                data->files_modified <= 1 ? "file" : "files");
    }
#endif

#ifdef _WIN32
    if (data->new_files && ((int)(data->new_files - data->ignored) > 0)) {
        fprintf(stdout, " %llu %s added\n", data->new_files - data->ignored,
                (data->new_files - data->ignored) <= 1 ? "file" : "files");
        fprintf(stdout,
            "    (Use '"YELLOW PEG_NAME " commit"RESET"' to commit the changes)\n");
        fprintf(stdout,
            "    (Use '"YELLOW PEG_NAME " reset"RESET"' to unstage the changes)\n");
    }

    if (data->ignored) {
        fprintf(stdout,
                "Staged files exists, please commit or reset the changes\n");
        fprintf(stdout, " %llu %s ignored, already staged\n", data->ignored,
                data->ignored <= 1 ? "file" : "files");
        fprintf(stdout,
            "    (Use '"YELLOW PEG_NAME " commit"RESET"' to commit the changes)\n");
        fprintf(stdout,
            "    (Use '"YELLOW PEG_NAME " reset'"RESET" to unstage the changes)\n");
    }
#else
    if (data->new_files && (data->new_files - data->ignored)) {
        fprintf(stdout, " %zu %s added\n", data->new_files - data->ignored,
                (data->new_files - data->ignored) <= 1 ? "file" : "files");
        fprintf(stdout,
            "    (Use '"YELLOW PEG_NAME " commit"RESET"' to commit the changes)\n");
        fprintf(stdout,
            "    (Use '"YELLOW PEG_NAME " reset"RESET"' to unstage the changes)\n");
    }

    if (data->ignored) {
        fprintf(stdout,
                "Staged files exists, please commit or reset the changes\n");
        fprintf(stdout, " %zu %s ignored, already staged\n", data->ignored,
                data->ignored <= 1 ? "file" : "files");
        fprintf(stdout,
            "    (Use '"YELLOW PEG_NAME " commit"RESET"' to commit the changes)\n");
        fprintf(stdout,
            "    (Use '" PEG_NAME " reset' to unstage the changes)\n");
    }
#endif
}

void load_old_cache_file(struct cache_object *co) { cache_object_init(co); }

bool is_marked_as_ignored(const char *name)
{
    struct strbuf temp = STRBUF_INIT;
    for (int i = 0; i < opts.ignore; i++) {
        if (name[0] != '.')
            strbuf_addstr(&temp, "./");
        strbuf_addbuf(&temp, &opts.ignarr[i]);
        if (!strncmp(name, opts.ignarr[i].buf, opts.ignarr[i].len - 1) ||
            !strncmp(name, temp.buf, temp.len - 1)) {
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

    if (is_marked_as_ignored(name)) {
        return 1;
    }
    filespec_init(&fs, name, "r");
    filespec_read_safe(&fs, &a);
    if (opts.more_output) {
        fprintf(stdout, GREEN "\t%s\n" RESET, name);
    }
    if (!read_file_from_database(name, &b)) {
        stats.new_files++;
        node = MALLOC(struct file_list, 1);
        if (!node) die("no memory available.\n");
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
        filespec_free(&fs);
        return 0;
    }
    if (strbuf_cmp(&a, &b)) {
        stats.files_modified++;
        node = MALLOC(struct file_list, 1);
        if (!node) die("no memory available.\n");
        node->old = true;
        strbuf_init(&node->path, 0);
        strbuf_addstr(&node->path, name);
        strbuf_release(&b);
        node->file.buf = a.buf;
        node->file.len = a.len;
        node->file.alloc = a.alloc;
        node->next = NULL;
        file_list_add(node);
        filespec_free(&fs);
        return 0;
    }
    filespec_free(&fs);
    strbuf_release(&a);
    strbuf_release(&b);
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

static inline void print_file_list_node(struct file_list *node)
{
    printf(YELLOW);
    printf("\t%s %s\n", node->old ? "M" : "N", node->path.buf);
    fflush(stdout);
    printf(RESET);
}

void cache_files()
{
    struct cache_object cache;
    struct file_list *node = head;

    cache_object_init(&cache);
    while (node) {
        if (!find_file_from_cache(node->path.buf, &cache)) {
            if (opts.verbose) print_file_list_node(node);
            process_node(&cache, node);
        } else {
            if (opts.verbose) {
                printf(CYAN);
                printf(" ignored: %s: already staged\n", node->path.buf);
                printf(RESET);
            }
            stats.ignored++;
        }
        node = node->next;
    }
    if (stats.total && opts.verbose) {
        printf("\n");
    }
    cache_object_write(&cache);

    /* a little memory management is required here */
    return;
}

int detect_and_add_files(const char *dir)
{
    size_t count;

    count = for_each_file_in_directory_recurse(dir, is_modified);
    stats.total = count;
    return 0;
}

void print_stage_usage() { fprintf(stdout, "%s", stage_usage); }

size_t findch(char *a, char delim)
{
    char *temp = a;
    while (*a++ != '\0')
        if (*a == delim) return a - temp;
    return -1;
}

void parse_ignore_list(char *argv)
{
    struct strbuf buf = STRBUF_INIT;
    strbuf_addstr(&buf, argv + findch(argv, '=') + 1);

    size_t size = strbuf_count(&buf, ':') + 1;
    size_t i = 0;
    opts.ignarr = MALLOC(struct strbuf, size);
    opts.ignore = size;
    char *tok = strtok(buf.buf, ":");

    while (tok != NULL) {
        strbuf_init(opts.ignarr + i, 128);
        strbuf_addstr(opts.ignarr + i, tok);
        tok = strtok(NULL, ":");
        i++;
    }
}

void process_argument(int i, char *argv)
{
    int ret;
    struct strbuf path = STRBUF_INIT;

    if (i == 1 && !strcmp(argv, ".")) {
        opts.all = 1;
    } else if (!strcmp(argv, "-h") || !strcmp(argv, "--help")) {
        print_stage_usage();
        exit(0);
    } else if (!strcmp(argv, "-a") || !strcmp(argv, "--all")) {
        opts.all = 1;
    } else if (!strcmp(argv, "-v") || !strcmp(argv, "--verbose")) {
        opts.verbose = 1;
    } else if (!strcmp(argv, "-m") || !strcmp(argv, "--more")) {
        opts.more_output = 1;
    } else if (!strncmp(argv, "-i", 2) || !strncmp(argv, "--ignore", 8)) {
        parse_ignore_list(argv);
    } else if (!opts.all) {
        ret = is_valid_path(argv);
        switch (ret) {
        case _FILE_:
            get_peg_path_buf(&path, argv);
            if (!is_modified(path.buf)) stats.total++;
            break;

        case _DIRECTORY_:
            get_peg_path_buf(&path, argv);
            detect_and_add_files(path.buf);
            break;

        default:
            fatal("%s: %s\n", argv, strerror(errno));
            exit(0);
        }
    }
}

int parse_arguments(int argc, char *argv[])
{
    if (argc < 2) {
        die("atleast one argument is required.\n"
            "    (See --help or -h)\n");
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
        detect_and_add_files(".");
    }
    cache_files();
    print_stage_stats(&stats);
    return 0;
}
