#include "delta.h"
#include "commit.h"
#include "path.h"

int delta_table_init(struct delta_table *table, int col, int row)
{
    table->table = MALLOC(enum arrow_t *, row);
    if (!table->table) {
        die("Out of memory. Can't make table:%d\n", row);
        return -1;
    }

    for (int i = 0; i <= row; i++) {
        table->table[i] = MALLOC(enum arrow_t, col);
        if (!table->table[i]) {
            die("Out of memory. Can't make a col:%d\n", col);
            return -1;
        }
        // memset(table->table[i], DELTA_DOWN, sizeof(enum arrow_t) * col);
    }

    table->prev = (int *)malloc(sizeof(int) * col);
    table->sol = (int *)malloc(sizeof(int) * col);

    if (!table->prev || !table->sol) {
        die("Out of memory. Can't make a table:%d\n", col);
    }

    memset(table->prev, 0, sizeof(int) * (col));
    memset(table->sol, 0, sizeof(int) * (col));

    table->row = row;
    table->col = col;
    return 0;
}

void delta_table_free(struct delta_table *table)
{
    // free(table->table);
    // free(table->prev);
    // free(table->sol);
}

int delta_input_init(struct delta_input *di, struct filespec *fs1,
                     struct filespec *fs2)
{
    di->fs1 = fs1;
    di->fs2 = fs2;

    if (deltafile_init_filespec(&di->df1, fs1, DELIM) < 0) return -1;

    if (deltafile_init_filespec(&di->df2, fs2, DELIM) < 0) return -1;

    return 0;
}

void delta_input_free(struct delta_input *di)
{
    deltafile_free(&di->df1);
    deltafile_free(&di->df2);
}

void print_table(const char *str, int *t, int len)
{
    printf("%s: ", str);
    for (int i = 0; i <= len; i++) {
        printf("%d ", t[i]);
    }
}

size_t delta_basic_comparison_m(struct delta_table *out, struct deltafile *af,
                                struct deltafile *bf)
{
    int i = 0;
    int j = 0;
    int prev = 0;
    int sol = 0;
    int m = out->col - 1;
    int n = out->row - 1;

    while (++i <= n) {
        while (++j <= m) {
            if (!inplace_compare(af->file.buf + af->arr[j - 1] + 1,
                                 bf->file.buf + bf->arr[i - 1] + 1,
                                 af->arr[j] - af->arr[j - 1],
                                 bf->arr[i] - bf->arr[i - 1])) {
                out->sol[j] = prev + 1;
                out->table[i][j] = DELTA_TILT;
            } else if (prev < sol) {
                out->sol[j] = sol;
                out->table[i][j] = DELTA_LEFT;
            } else {
                out->sol[j] = out->prev[j];
                out->table[i][j] = DELTA_UP;
            }

            prev = out->prev[j];
            out->prev[j] = out->sol[j];
            sol = out->sol[j];
        }
#if DEBUG
        print_table("", out->prev, m);
        printf("\t%d, %d", i, j);
        printf("\n");
#endif

        prev = out->prev[0];
        sol = out->sol[0];

        j = 0;
    }

    return out->sol[m];
}

void basic_delta_result_init(struct basic_delta_result *bdr,
                             struct delta_input *di)
{
    bdr->input = di;
    bdr->insertions = 0;
    bdr->deletions = 0;
    bdr->common = 0;
    strbuf_list_init(&bdr->common_lines);
    strbuf_list_init(&bdr->diff_lines);
}

int delta_backtrace_table(struct basic_delta_result *result,
                          struct delta_table *table, struct deltafile *af,
                          struct deltafile *bf)
{
    result->common = table->sol[table->col - 1];

    int i, j; // index of the current cell
    enum arrow_t **tab = table->table;
    int row = table->row, col = table->col;
    struct strbuf *a = &af->file;
    struct strbuf *b = &bf->file;

    i = row - 1;
    j = col - 1;
    while (i != 0 && j != 0) {
        switch (tab[i][j]) {
        case DELTA_UP:
            result->deletions++;
            strbuf_list_add(&result->diff_lines, b->buf + bf->arr[i - 1] + 1,
                            bf->arr[i] - bf->arr[i - 1], '-', i);
            i--;
            break;

        case DELTA_LEFT:
            result->insertions++;
            strbuf_list_add(&result->diff_lines, a->buf + af->arr[j - 1] + 1,
                            af->arr[j] - af->arr[j - 1], '+', j);
            j--;
            break;

        case DELTA_TILT:
            strbuf_list_add(&result->common_lines, a->buf + af->arr[j - 1] + 1,
                            af->arr[j] - af->arr[j - 1], '~', j);
            j--;
            i--;
            break;

        default:
            die("BUG: some error occurred in delta\n");
        }
    }
    return 0;
}

int delta_backtrace_table_minimal(struct basic_delta_result *result,
                                  struct delta_table *table,
                                  struct deltafile *af, struct deltafile *bf)
{
    result->common = table->sol[table->col - 1];

    int i, j; // index of the current cell
    enum arrow_t **tab = table->table;
    int row = table->row, col = table->col;

    i = row - 1;
    j = col - 1;
    while (i != 0 && j != 0) {
        switch (tab[i][j]) {
        case DELTA_UP:
            result->deletions++;
            i--;
            break;
        case DELTA_LEFT:
            result->insertions++;
            j--;
            break;
        case DELTA_TILT:
            j--;
            i--;
            break;
        default:
            die("BUG: some error occurred in delta\n");
        }
    }
    return 0;
}

void delta_stat(struct basic_delta_result *bdr, struct strbuf *stat)
{
    if (bdr->input)
        strbuf_addf(stat, "@@@ delta %s %s @@@\n", bdr->input->fs1->fname.buf,
                    bdr->input->fs2->fname.buf);
    if ((bdr->insertions == 0) && (bdr->deletions == 0)) {
        strbuf_addf(stat, "No difference\n");
        return;
    }

    if (bdr->insertions || bdr->deletions == 0) {
        strbuf_addf(stat, "%llu %s ", bdr->insertions,
                    (bdr->insertions == 1) ? "insertion(+)" : "insertions(+)");
    }
    if (bdr->deletions || bdr->insertions == 0) {
        strbuf_addf(stat, "%llu %s ", bdr->deletions,
                    (bdr->deletions == 1) ? "deletion(-)" : "deletions(-)");
    }
    strbuf_addch(stat, '\n');
    return;
}

void delta_summary(struct basic_delta_result *bdr, struct strbuf *summary)
{
    struct strbuf_list_node *node;
    node = bdr->diff_lines.head->next;
    while (node != NULL) {
        printf("%c", node->sign);
        printf("%s", node->buf.buf);
        node = node->next;
    }
}

struct delta_stat {
    size_t insertions;
    size_t deletions;
};

bool strbuf_delta_minimal(struct strbuf *out, struct basic_delta_result *result,
                          struct strbuf *b, struct strbuf *a)
{
    struct deltafile af, bf;
    struct delta_table table;

    deltafile_init_strbuf(&af, a, DELIM);
    deltafile_init_strbuf(&bf, b, DELIM);
    delta_table_init(&table, af.size, bf.size);
    delta_basic_comparison_m(&table, &af, &bf);
    delta_backtrace_table_minimal(result, &table, &af, &bf);
    if (!(result->insertions || result->deletions)) return false;
    if (out) delta_stat(result, out);
    return true;
}

bool strbuf_delta_enhanced(struct strbuf *out,
                           struct basic_delta_result *result, struct strbuf *b,
                           struct strbuf *a)
{
    struct strbuf_list_node *node;
    struct deltafile af, bf;
    struct delta_table table;

    deltafile_init_strbuf(&af, a, DELIM);
    deltafile_init_strbuf(&bf, b, DELIM);
    delta_table_init(&table, af.size, bf.size);
    delta_basic_comparison_m(&table, &af, &bf);
    delta_backtrace_table(result, &table, &af, &bf);
    if (!(result->insertions || result->deletions)) return false;

    node = result->diff_lines.head->next;
    while (node) {
        if (node->sign == '+') {
            strbuf_addstr(out, GREEN);
        }
        else {
            strbuf_addstr(out, RED);
        }
        strbuf_addch(out, node->sign);
        strbuf_addstr(out, RESET);
        strbuf_add(out, node->buf.buf, node->buf.len);
        node = node->next;
    }
    return true;
}

void index_delta(struct basic_delta_result *result,
                 struct pack_file_cache *cache, struct index *i1,
                 struct index *i2, struct strbuf *out, bool minimal)
{
    struct strbuf a = STRBUF_INIT;
    struct strbuf b = STRBUF_INIT;
    struct basic_delta_result local;

    basic_delta_result_init(&local, NULL);
    strbuf_add(&a, cache->cache.buf + i1->pack_start, i1->pack_len);
    strbuf_add(&b, cache->cache.buf + i2->pack_start, i2->pack_len);
    if (minimal) {
        if (!strbuf_delta_minimal(out, &local, &a, &b)) {
            strbuf_list_free(&local.common_lines);
            strbuf_list_free(&local.diff_lines);
            strbuf_release(&a);
            strbuf_release(&b);
            return;
        }
        result->insertions += local.insertions;
        result->deletions += local.deletions;
    } else {
        strbuf_delta_enhanced(out, result, &a, &b);
        strbuf_list_append(&result->diff_lines, &local.diff_lines);
        strbuf_list_append(&result->common_lines, &local.common_lines);
        result->insertions += local.insertions;
        result->deletions += local.deletions;
    }

    strbuf_release(&a);
    strbuf_release(&b);
}

void delta_index_splash(struct strbuf *out, const char *i, const char *j)
{
    /*
     * if i is NULL, this means j was newly added.
     */
    if (i || !j) strbuf_addf(out, "%s", i);
    if (i && j) strbuf_addf(out, " <--> ");
    if (j || !i) strbuf_addf(out, "%s", j);
    if (!i && !j) {
        fprintf(stderr, "%s\n", out->buf);
        die("BUG: i & j both were NULL");
    }
    strbuf_addch(out, '\n');
}

void print_lines(struct strbuf *buf, bool i_or_d)
{
    size_t lines = count_lines(buf);
    i_or_d ? fprintf(stdout, "%llu %s\n", lines,
                     lines == 1 ? "insertion" : "insertions")
           : fprintf(stdout, "%llu %s\n", lines,
                     lines == 1 ? "deletion" : "deletions");
}

void print_insertion_lines(struct strbuf *buf)
{
    size_t lines = count_lines(buf);
    struct deltafile file;
    struct strbuf out = STRBUF_INIT;

    deltafile_init_strbuf(&file, buf, DELIM);
    for (int i = 0; i < file.size - 1; i++) {
        strbuf_addstr(&out, GREEN"+"RESET);
        strbuf_add(&out, buf->buf + file.arr[i] + 1,
                         file.arr[i + 1] - file.arr[i]);
    }
    fprintf(stdout, "%s\n", out.buf);
    strbuf_release(&out);
}

void print_insertion_only(struct pack_file_cache *cache, struct index *idx)
{
    struct strbuf temp = STRBUF_INIT;
    temp.len = idx->pack_len;
    temp.buf = cache->cache.buf + idx->pack_start;
    temp.alloc = count_lines(&temp);
    fprintf(stdout, "%llu %s", temp.alloc,
            temp.alloc == 1 ? "insertion\n" : "insertions\n");
}

void print_object_insertions(struct pack_file_cache *cache, struct index *idx)
{
    struct strbuf temp = STRBUF_INIT, out = STRBUF_INIT;
    struct deltafile file;

    strbuf_add(&temp, cache->cache.buf + idx->pack_start, idx->pack_len);
    deltafile_init_strbuf(&file, &temp, DELIM);
    for (int i = 0; i < file.size - 1; i++) {
        strbuf_addstr(&out, GREEN"+"RESET);
        strbuf_add(&out, temp.buf + file.arr[i] + 1,
                         file.arr[i + 1] - file.arr[i]);
    }
    fprintf(stdout, "%s\n", out.buf);
    strbuf_release(&out);
    strbuf_release(&temp);
    deltafile_free(&file);
}

void print_deletion_lines(struct strbuf *buf)
{
    size_t lines = count_lines(buf);
    struct deltafile file;
    struct strbuf out = STRBUF_INIT;

    deltafile_init_strbuf(&file, buf, DELIM);
    for (int i = 0; i < file.size - 1; i++) {
        strbuf_addstr(&out, RED"-"RESET);
        strbuf_add(&out, buf->buf + file.arr[i] + 1,
                         file.arr[i + 1] - file.arr[i]);
    }
    fprintf(stdout, "%s\n", out.buf);
    strbuf_release(&out);
    deltafile_free(&file);
}

void print_deletion_only(struct pack_file_cache *cache, struct index *idx)
{
    struct strbuf temp = STRBUF_INIT;
    temp.len = idx->pack_len;
    temp.buf = cache->cache.buf + idx->pack_start;
    temp.alloc = count_lines(&temp);
    fprintf(stdout, "%llu %s\n", temp.alloc,
            temp.alloc == 1 ? "deletion" : "deletions");
}

void do_commit_delta(struct commit *c1, struct commit *c2, bool minimal)
{
    struct index_list *a = NULL, *b = NULL;
    struct index_list *nodea;
    struct index *bi;
    struct basic_delta_result result;
    struct strbuf out = STRBUF_INIT;
    struct pack_file_cache cache = PACK_FILE_CACHE_INIT;

    basic_delta_result_init(&result, NULL);
    make_index_list_from_commit(c1, &a);
    make_index_list_from_commit(c2, &b);
    if (!a || !b) {
        error("\n");
        return;
    }
    nodea = a;
    cache_pack_file(&cache);
    while (nodea) {
        bi = find_file_index_list(b, nodea->idx->filename);
        /* NOTE: bi can be NULL also */
        if (bi)
            delta_index_splash(&out, nodea->idx->filename, bi->filename);
        else
            delta_index_splash(&out, nodea->idx->filename, NULL);
        bi->flags = DELTA_FLAG;
        index_delta(&result, &cache, nodea->idx, bi, &out, minimal);
        fwrite(out.buf, sizeof(char), out.len, stdout);
        strbuf_setlen(&out, 0);
        nodea = nodea->next;
    }
    nodea = b;
    while (nodea) {
        if (nodea->idx->flags == DELTA_FLAG) {
            nodea = nodea->next;
            continue;
        }
        delta_index_splash(&out, NULL, nodea->idx->filename);
        fwrite(out.buf, 1, out.len, stdout);
        if (!minimal)
            print_insertion_only(&cache, nodea->idx);
        else {
        }
        strbuf_setlen(&out, 0);
    }
    strbuf_release(&cache.cache);
}

void do_file_delta_minimal(const char *path, struct strbuf *a, struct strbuf *b)
{
    struct strbuf out = STRBUF_INIT;
    struct basic_delta_result result;

    basic_delta_result_init(&result, NULL);
    delta_index_splash(&out, path, path);
    strbuf_delta_minimal(&out, &result, a, b);
    fprintf(stdout, "%s", out.buf);
    strbuf_release(&out);
}

void do_file_delta_enhanced(const char *path, struct strbuf *a,
                            struct strbuf *b)
{
    struct strbuf out = STRBUF_INIT;
    struct basic_delta_result result;

    basic_delta_result_init(&result, NULL);
    delta_index_splash(&out, path, path);
    strbuf_delta_enhanced(&out, &result, a, b);
    fprintf(stdout, "%s", out.buf);
    strbuf_release(&out);
}

/*
 * compares the single file specified by its path with its HEAD's copy.
 */
void do_single_file_delta(const char *path, bool minimal)
{
    struct strbuf buf = STRBUF_INIT;
    struct strbuf filebuf = STRBUF_INIT;
    FILE *file = fopen(path, "r");
    bool result = find_file_from_head_commit(path, &buf);

    if (result && file) {
        /* both file exists */
        strbuf_fread(&filebuf, file_length(file), file);
        minimal ? do_file_delta_minimal(path, &buf, &filebuf)
                : do_file_delta_enhanced(path, &buf, &filebuf);

    } else if (result && !file) {
        /* original file is deleted */
        delta_index_splash(&filebuf, path, NULL);
        fprintf(stdout, "%s", filebuf.buf);
        minimal ? print_lines(&buf, 0) : print_deletion_lines(&buf);
    } else if (!result && file) {
        /* new file is added */
        delta_index_splash(&buf, NULL, path);
        fprintf(stdout, "%s", buf.buf);
        strbuf_fread(&filebuf, file_length(file), file);
        minimal ? print_lines(&filebuf, 1) : print_insertion_lines(&filebuf);
    } else {
        /* none of the file exists */
        die("%s: file doesn't exists\n", path);
    }

    if (file) fclose(file);
    strbuf_release(&filebuf);
    strbuf_release(&buf);
}

/*
 * commit_delta: compares two given commits
 */
void commit_delta(char commit1_sha[HASH_SIZE], char commit2_sha[HASH_SIZE],
                  bool minimal)
{
    struct commit_list *cl = NULL;
    struct commit *a, *b;

    /*
     * In case both commits were same.
     */
    if (!strcmp(commit1_sha, commit2_sha)) return;
    make_commit_list(&cl);
    if ((a = find_commit_hash(cl, commit1_sha)) == NULL) {
        fatal("commit <");
        print_hash(commit1_sha, stderr);
        printf("> doesn't exists\n");
        exit(-1);
    }
    if ((b = find_commit_hash(cl, commit2_sha)) == NULL) {
        fatal("commit <");
        print_hash(commit2_sha, stderr);
        printf("> doesn't exists\n");
        exit(-1);
    }
    do_commit_delta(a, b, minimal);
}

/*
 * In single commit delta we compare one given commit with the HEAD commit
 */
void do_single_commit_delta(char *commit1_hash, bool minimal, bool noconv)
{
    struct commit_list *cl;
    struct commit *a, *b;
    char real_sha1[HASH_SIZE];
    size_t len = HASH_SIZE;

    make_commit_list(&cl);
    if (!noconv) {
        len = char_to_sha1(real_sha1, commit1_hash);
    }

    if ((a = find_commit_hash_compat(cl, real_sha1, len)) == NULL) {
        fatal("commit <");
        print_hash_size(real_sha1, len, stderr);
        fprintf(stdout, "> doesn't exists.\n");
        exit(-1);
    }
    b = get_head_commit(cl);
    do_commit_delta(a, b, minimal);
}

struct delta_options {
    bool minimal;
    bool recursive;
    bool file;
    bool commit;
    bool help;
    bool guessed;
    char *hash_arg1;
    char *hash_arg2;
    char *file_name;
};

#define DELTA_OPTIONS_DEFAULT                                                  \
    {                                                                          \
        0, 0, 0, 0, 0, 0, NULL, NULL                                           \
    }

/**
 * following options will be provided by delta for now:
 *  -m, --minimal : for minimal comparison
 *  -h, --hash : (optional) specified explicitly that we're doing commit delta
 *  -f, --file : (optional) specifying explicitly that we're doing file delta
 *
 * TODO: following options will implemented later:
 *  -r, --recursive: recursively compare the directory contents
 *  -i, --insertions: show insertions only
 *  -d, --deletions: show deletions only
 *  -c, --cached: show delta of the staged files
 *  --no-color:
 */

#define is(option) !strcmp(option, argv[count])

void delta_parse_single_option(struct delta_options *opts, int count,
                               char *argv[])
{
    struct stat st;
    struct strbuf path = STRBUF_INIT;

    if (is("--help") || is("-h"))
        opts->help = true;
    else if (is("-m") || is("--minimal"))
        opts->minimal = true;
    else if (is("--hash"))
        opts->commit = true;
    else if (is("--file") || is("-f"))
        opts->file = true;
    else if (opts->file && !opts->commit) {
        opts->file_name = argv[count];
        if (stat(opts->file_name, &st) < 0) {
            fatal("%s: %s\n", opts->file_name,
                    strerror(errno));
            exit(-1);
        }
        if (S_ISDIR(st.st_mode))
            opts->recursive = true;
        else if (S_ISREG(st.st_mode))
            opts->file = true;
        else
            die("%s: file is of unknown type.\n", argv[count]);
    } else if (opts->commit && !opts->file) {
        if (!is_valid_hash(argv[count], strlen(argv[count])))
            die("%s: not a valid sha1\n", argv[count]);
        opts->hash_arg1 ? (opts->hash_arg2 = argv[count])
                        : (opts->hash_arg1 = argv[count]);
    } else if (!opts->commit || !opts->file) {
        if (stat(argv[count], &st) < 0) {
            // now it can be a sha, check for its validity
            if (!is_valid_hash(argv[count], strlen(argv[count]))) {
                fatal("fatal: %s: ", opts->file_name);
                if (errno == EPERM)
                    fprintf(stderr, "permission denied\n");
                else if (errno == ENOENT)
                    fprintf(stderr, "file or directory doesn't exists\n");
                else
                    fprintf(stderr, "unknown error occurred\n");
                exit(-1);
            }
            opts->commit = true;
            opts->hash_arg1 ? (opts->hash_arg2 = argv[count])
                            : (opts->hash_arg1= argv[count]);
            return;
        }
        if (S_ISDIR(st.st_mode)) {
            opts->recursive = true;
            get_peg_path_buf(&path, argv[count]);
            opts->file_name = path.buf;
        } else if (S_ISREG(st.st_mode)) {
            opts->file = true;
            get_peg_path_buf(&path, argv[count]);
            opts->file_name = path.buf;
        } else
            die("%s: file is of unknown type.\n", argv[count]);
        opts->guessed = true;
    } else {
        die("invalid option `%s'\n", argv[count]);
    }
}

void delta_parse_options(struct delta_options *opts, int argc, char *argv[])
{
    if (argc < 2) {
        struct commit_list *cl;
        make_commit_list(&cl);
        struct commit *cm = get_head_commit(cl);

        do_single_commit_delta(cm->sha1, opts->minimal, true);
    }
    for (int i = 1; i < argc; i++) {
        delta_parse_single_option(opts, i, argv);
    }
}

void delta_main(int argc, char *argv[])
{
    struct delta_options opts = DELTA_OPTIONS_DEFAULT;

    delta_parse_options(&opts, argc, argv);
    if (opts.help) {
        printf("Usage: delta (options) (commits) | (commit) | (path)\n");
        printf("\n");
        printf("options: \n");
        printf(" (-r | --recursive) | (-h | --help) | (--hash) | (--file)\n");
        exit(-1);
    }
    if (opts.commit && !opts.file) {
        if (opts.hash_arg1 && opts.hash_arg2) {
            commit_delta(opts.hash_arg1, opts.hash_arg2, opts.minimal);
        } else if (opts.hash_arg1) {
            do_single_commit_delta(opts.hash_arg1, opts.minimal, 1);
        } else if (opts.hash_arg2) {
            do_single_commit_delta(opts.hash_arg2, opts.minimal, 1);
        } else if (opts.guessed) {
            die("no sha1 or path specified.\n");
        } else {
            die("please provide atleast one argument for sha1.\n");
        }
    } else if (!opts.commit && opts.file) {
        if (opts.recursive) {
            die("not implemented yet.\n");
        } else if (!opts.file_name && opts.guessed) {
            die("no sha1 or path specified.\n");
        } else if (opts.guessed || opts.file_name) {
            do_single_file_delta(opts.file_name, opts.minimal);
        } else {
            die("no path specified.\n");
        }
    } else {
        struct commit_list *cl;
        make_commit_list(&cl);
        struct commit *cm = get_head_commit(cl);

        do_single_commit_delta(cm->sha1, opts.minimal, true);
    }
}
