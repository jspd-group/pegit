#include "stage.h"
#include "sha1-inl.h"
#include "tree.h"
#include "file.h"
#include "path.h"
#include "util.h"

struct stage_options {
    int all;
    int ignore;
    int all_dot;
    int verbose;
    size_t bytes;
    bool modified_only;
    int more_output;
    struct strbuf *ignarr;
    struct strbuf *add;
    struct index_list *head;
    struct pack_file_cache cache;
};

char stage_usage[] = CYAN PEG_NAME
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
    size_t deleted;
    size_t ignored;
    size_t total;
} stats = {0, 0, 0, 0};


struct file_list *last, *head;
struct stage_options opts = {
    .cache = PACK_FILE_CACHE_INIT
};

void file_list_clear(struct file_list *head)
{
    struct file_list *node = head;
    struct file_list *temp = node;

    while (temp) {
        node = node->next;
        strbuf_release(&temp->path);
        strbuf_release(&temp->file);
        free(temp);
        temp = node;
    }
}

void show_node(struct cache_index_entry_list *node, size_t i)
{
    printf(SIZE_T_FORMAT "\t" SIZE_T_FORMAT "\t" SIZE_T_FORMAT "\t%s\n",
        i, node->start, node->len, node->file_path.buf);
}

void show_cache_table()
{
    struct cache_object co;
    struct cache_index_entry_list *node;
    size_t i = 0;

    cache_object_init(&co);
    node = co.ci.entries;
    while (node) {
        if (!i)
            printf("INDEX\tI_START\tI_SIZE\tFILE_NAME\n");
        show_node(node, i++);
        node = node->next;
    }
}

int read_file_from_database(const char *path, struct strbuf *buf)
{
    struct index *idx;

    if (!opts.head)
        return 0;

    idx = find_file_index_list(opts.head, path);
    if (!idx) return false;

    return !get_file_content(&opts.cache, buf, idx);
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
    opts.bytes = 0;
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
    fl->status = 0;
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
    if (data->files_modified && (((ssize_t)(data->files_modified - ignored)) > 0)) {
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
    size_t len = strlen(name);

    for (int i = 0; i < opts.ignore; i++) {
        if (name[0] != '.' && len >= 2 && name[1] != '/')
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
    struct stat st;

    if (is_marked_as_ignored(name)) {
        return 1;
    }
    if (!read_file_from_database(name, &b)) {
        if (opts.modified_only) {
            filespec_free(&fs);
            return 0;
        }

        if (stat(name, &st) < 0) {
            stats.deleted++;
            node = MALLOC(struct file_list, 1);
            if (!node) die("no memory available.\n");
            file_list_init(node);
            node->status = DELETED;
            strbuf_init(&node->path, 0);
            strbuf_addstr(&node->path, name);
            strbuf_init(&node->file, 0);

            node->next = NULL;
            file_list_add(node);
            return 0;
        }

        filespec_init(&fs, name, "r");
        filespec_read_safe(&fs, &a);

        stats.new_files++;
        node = MALLOC(struct file_list, 1);
        if (!node) die("no memory available.\n");
        file_list_init(node);
        node->status = NEW;
        node->st = st;
        strbuf_init(&node->path, 0);
        strbuf_addstr(&node->path, name);

        node->file.buf = a.buf;
        node->file.len = a.len;
        node->file.alloc = a.alloc;
        opts.bytes += a.len;

        if (opts.more_output) {
            printf("\r");
            printf("\t\t\t\t\r");
            print_humanised_bytes(opts.bytes);
        }
        node->next = NULL;
        file_list_add(node);
        filespec_free(&fs);
        return 0;

    } else if (strbuf_cmp(&a, &b)) {

        if (stat(name, &st) < 0) {
            die("unable to stat %s, %s\n", name, strerror(errno));
        }

        stats.files_modified++;
        node = MALLOC(struct file_list, 1);
        if (!node) die("no memory available.\n");
        node->status = MODIFIED;
        strbuf_init(&node->path, 0);
        strbuf_addstr(&node->path, name);
        strbuf_release(&b);
        node->file.buf = a.buf;
        node->file.len = a.len;
        node->file.alloc = a.alloc;
        opts.bytes += a.len;

        if (opts.more_output) {
            printf("\r");
            print_humanised_bytes(opts.bytes);
        }
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

    n->len = node->file.len;
    strbuf_init(&n->file_path, 0);
    strbuf_addbuf(&n->file_path, &node->path);
    n->st = node->st;
    n->status = node->status;
    n->next = NULL;
    cache_object_addindex(cache, &node->file, n);
}

static inline void print_file_list_node(struct file_list *node)
{
    printf(YELLOW);
    printf(" %s   %s\n", node->status == MODIFIED ? "M" : "N", node->path.buf);
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
    file_list_clear(head);
    cache_object_write(&cache);

    /* a little memory management is required here */
    return;
}

int detect_and_add_files(const char *dir)
{
    status((char*)dir);
    struct file_list *node;
    struct node *st_node = root;
    struct stat st;
    int fd = 0;
    ssize_t r = 0;

    while (st_node) {
        if (s_is_modified(st_node)) {
            node = malloc(sizeof(struct file_list));
            strbuf_init(&node->path, 0);
            strbuf_addstr(&node->path, st_node->name);

            if (stat(node->path.buf, &node->st) < 0) {
                die("unable to stat %s, %s\n", node->path.buf, strerror(errno));
            }
            if (opts.verbose)
                printf("opening %s\n", node->path.buf);
            fd = open(node->path.buf, O_RDONLY);
            if (fd < 0) {
                die("unable to read %s, %s\n", node->path.buf, strerror(errno));
            }
            strbuf_init(&node->file, node->st.st_size);
            r = read(fd, node->file.buf, node->st.st_size);
            if (r < 0) {
                die("read returned %lld\n", r);
            }
            opts.bytes += node->st.st_size;
            node->file.len = r;
            node->status = st_node->status;
            node->next = NULL;
            file_list_add(node);
            if (close(fd) < 0)
                die("unable to close %s, %s\n", node->path.buf, strerror(errno));
            if (opts.more_output) {
                printf("\t\t\t\r");
                print_humanised_bytes(opts.bytes);
            }
        }
        st_node = st_node->next;
    }

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
    size_t i = 0, size = 0;
    struct strbuf buf = STRBUF_INIT;
    struct strbuf entry = STRBUF_INIT;
    strbuf_addstr(&buf, argv + findch(argv, '=') + 1);

    size = strbuf_count(&buf, ':') + (buf.buf[buf.len - 1] == ':' ? 0 : 1);
    opts.ignarr = MALLOC(struct strbuf, size);
    opts.ignore = size;
    char *tok = strtok(buf.buf, ":");

    while (tok != NULL) {
        strbuf_init(opts.ignarr + i, 128);
        get_peg_path_buf(opts.ignarr + i, tok);
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
    } else if (!strcmp(argv, "-m") || !strcmp(argv, "--modified")) {
        opts.modified_only = true;
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
        status(".");
        detect_and_add_files(".");
    }
    cache_files();
    print_stage_stats(&stats);
    return 0;
}
