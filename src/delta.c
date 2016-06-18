#include "delta.h"
#include "commit.h"
#include "path.h"
#include "pack.h"

#define DELTA_OPTIONS_DEFAULT                                                  \
    {                                                                          \
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL                         \
    }

struct delta_options {
    bool minimal;
    bool recursive;
    bool file;
    bool commit;
    bool verbose;
    bool help;
    bool guessed;
    bool original_diff;
    bool summary;
    bool debug;
    char *hash_arg1;
    char *hash_arg2;
    char *file_name;
} d_opts = DELTA_OPTIONS_DEFAULT;

enum delta_error {
    ALL_GOOD,
    MEM_ERROR,
    INTERNAL_ERROR
};

static void release_table(enum arrow_t **table, int size)
{
    while (size--) {
        free(table[size]);
        table[size] = NULL;
    }
    free(table);
}

int delta_table_init(struct delta_table *table, int col, int row)
{
    table->table = MALLOC(enum arrow_t *, row + 1);
    if (!table->table) {
        return MEM_ERROR;
    }

    for (int i = 0; i <= row; i++) {
        table->table[i] = MALLOC(enum arrow_t, col);
        if (!table->table[i]) {
            release_table(table->table, i);
            return MEM_ERROR;
        }
        // memset(table->table[i], DELTA_DOWN, sizeof(enum arrow_t) * col);
    }

    table->prev = malloc(sizeof(int) * col);
    table->sol = malloc(sizeof(int) * col);

    if (!table->prev || !table->sol) {
        release_table(table->table, row);
        if (table->prev)
            free(table->prev);
        else free(table->sol);
        return MEM_ERROR;
    }

    memset(table->prev, 0, sizeof(int) * (col));
    memset(table->sol, 0, sizeof(int) * (col));

    table->row = row;
    table->col = col;
    return ALL_GOOD;
}

void delta_table_free(struct delta_table *table)
{
    for (int i = 0; i <= table->row; i++) {
        free(table->table[i]);
    }
    free(table->table);
    free(table->prev);
    free(table->sol);
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

void basic_delta_result_release(struct basic_delta_result *bdr)
{
    strbuf_list_free(&bdr->common_lines);
    strbuf_list_free(&bdr->diff_lines);
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
#ifdef _WIN32
    if (bdr->insertions || bdr->deletions == 0) {
        strbuf_addf(stat, BOLD_GREEN SIZE_T_FORMAT RESET" %s ", bdr->insertions,
                    (bdr->insertions == 1) ? "insertion(+)" : "insertions(+)");
    }
    if (bdr->deletions || bdr->insertions == 0) {
        strbuf_addf(stat, BOLD_RED SIZE_T_FORMAT RESET" %s ", bdr->deletions,
                    (bdr->deletions == 1) ? "deletion(-)" : "deletions(-)");
    }
#else
    if (bdr->insertions || bdr->deletions == 0) {
        strbuf_addf(stat, BOLD_GREEN"%zu"RESET" %s ", bdr->insertions,
                    (bdr->insertions == 1) ? "insertion(+)" : "insertions(+)");
    }
    if (bdr->deletions || bdr->insertions == 0) {
        strbuf_addf(stat, BOLD_RED"%zu"RESET" %s ", bdr->deletions,
                    (bdr->deletions == 1) ? "deletion(-)" : "deletions(-)");
    }
#endif
    strbuf_addch(stat, '\n');
    return;
}

void print_delta_stat(struct basic_delta_result *bdr)
{
    size_t insertions = bdr->insertions;
    size_t deletions = bdr->deletions;

    if (insertions) {
        fprintf(stdout, SIZE_T_FORMAT" %s", insertions, insertions == 1 ?
            "insertion" : "insertions");
    }
    if (insertions && deletions) {
        printf(", ");
    }
    if (deletions) {
        fprintf(stdout, SIZE_T_FORMAT " %s", deletions, deletions == 1 ?
            "deletion" : "deletions");
    }
    putchar('\n');
}

void delta_summary(struct basic_delta_result *bdr, struct strbuf *summary)
{
    struct strbuf_list_node *node;
    node = bdr->diff_lines.head->next;

    (void) summary;
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
    struct basic_delta_result res;
    bool cont = false;

    deltafile_init_strbuf(&af, a, DELIM);
    deltafile_init_strbuf(&bf, b, DELIM);


    if (af.size == bf.size) {
        cont = strbuf_cmp(a, b);
        if (!cont) return false;
    }

    if (delta_table_init(&table, af.size, bf.size) == MEM_ERROR) {
        fprintf(stderr, "file too big\n");
        return false;
    }
    delta_basic_comparison_m(&table, &af, &bf);
    if (result) {
        delta_backtrace_table_minimal(result, &table, &af, &bf);
    }
    else {
        basic_delta_result_init(&res, NULL);
        delta_backtrace_table_minimal(result, &table, &af, &bf);
    }
    if (!(result->insertions || result->deletions)) {
        deltafile_free(&af);
        deltafile_free(&bf);
        return false;
    }
    if (out) delta_stat(result, out);
    deltafile_free(&af);
    deltafile_free(&bf);
    delta_table_free(&table);
    return true;
}

bool strbuf_delta_enhanced(struct strbuf *out,
                           struct basic_delta_result *result, struct strbuf *b,
                           struct strbuf *a)
{
    struct strbuf_list_node *node;
    struct deltafile af, bf;
    struct delta_table table;
    bool cont = false;

    deltafile_init_strbuf(&af, a, DELIM);
    deltafile_init_strbuf(&bf, b, DELIM);

    if (af.size == bf.size) {
        cont = strbuf_cmp(a, b);
        if (!cont) return false;
    }
    if (delta_table_init(&table, af.size, bf.size)) {
        perror("file too big");
        return false;
    }
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
        strbuf_add(out, node->buf.buf, node->buf.len);
        strbuf_addstr(out, RESET);
        node = node->next;
    }

    delta_table_free(&table);
    deltafile_free(&af);
    deltafile_free(&bf);
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
    get_file_content(cache, &a, i1);
    get_file_content(cache, &b, i2);
    if (minimal) {
        if (!strbuf_delta_minimal(out, &local, &a, &b)) {
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
    strbuf_addstr(out, YELLOW);
    if (i || !j) strbuf_addstr(out, i);
    if (i && j) strbuf_addstr(out, " <--> ");
    if (j || !i) strbuf_addstr(out, j);
    if (!i && !j) {
        fprintf(stderr, "%s\n", out->buf);
        die("BUG: i & j both were NULL");
    }
    strbuf_addstr(out, RESET);
    strbuf_addch(out, '\n');
}

size_t print_lines(struct strbuf *buf, bool i_or_d)
{
    size_t lines = count_lines(buf);

#ifdef _WIN32
    i_or_d ? fprintf(stdout, BOLD_GREEN  SIZE_T_FORMAT RESET" %s\n", lines,
                     lines == 1 ? "addition" : "additions")
           : fprintf(stdout, BOLD_RED SIZE_T_FORMAT RESET" %s\n", lines,
                     lines == 1 ? "deletion" : "deletions");
#else
    i_or_d ? fprintf(stdout, BOLD_GREEN "%zu"RESET" %s\n", lines,
                     lines == 1 ? "addition" : "additions")
           : fprintf(stdout, BOLD_RED"%zu"RESET" %s\n", lines,
                     lines == 1 ? "deletion" : "deletions");
#endif
    return lines;
}

void print_insertion_lines(struct strbuf *buf)
{
    size_t lines = count_lines(buf);
    struct deltafile file;
    struct strbuf out = STRBUF_INIT;

    deltafile_init_strbuf(&file, buf, DELIM);
    for (int i = 0; i < file.size - 1; i++) {
        strbuf_addstr(&out, GREEN"+");
        strbuf_add(&out, buf->buf + file.arr[i] + 1,
                         file.arr[i + 1] - file.arr[i]);
        strbuf_addstr(&out, RESET);
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
#ifdef _WIN32
    fprintf(stdout, BOLD_GREEN SIZE_T_FORMAT RESET" %s", temp.alloc,
            temp.alloc == 1 ? "addition\n" : "additions\n");
#else
    fprintf(stdout, BOLD_GREEN"%zu"RESET" %s", temp.alloc,
            temp.alloc == 1 ? "addition\n" : "additions\n");
#endif
}

void print_object_insertions(struct pack_file_cache *cache, struct index *idx)
{
    struct strbuf temp = STRBUF_INIT, out = STRBUF_INIT;
    struct deltafile file;

    strbuf_add(&temp, cache->cache.buf + idx->pack_start, idx->pack_len);
    deltafile_init_strbuf(&file, &temp, DELIM);
    for (int i = 0; i < file.size - 1; i++) {
        strbuf_addstr(&out, GREEN"+");
        strbuf_add(&out, temp.buf + file.arr[i] + 1,
                         file.arr[i + 1] - file.arr[i]);
        strbuf_addstr(&out, RESET);
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
        strbuf_addstr(&out, RED"-");
        strbuf_add(&out, buf->buf + file.arr[i] + 1,
                         file.arr[i + 1] - file.arr[i]);
        strbuf_addstr(&out, RESET);
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
#ifdef _WIN32
    fprintf(stdout, BOLD_RED SIZE_T_FORMAT RESET" %s\n", temp.alloc,
            temp.alloc == 1 ? "deletion" : "deletions");
#else
    fprintf(stdout, BOLD_RED"%zu"RESET" %s\n", temp.alloc,
            temp.alloc == 1 ? "deletion" : "deletions");
#endif
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
        if (bi) {
            delta_index_splash(&out, nodea->idx->filename, bi->filename);
            bi->flags = DELTA_FLAG;
            index_delta(&result, &cache, nodea->idx, bi, &out, minimal);
            fwrite(out.buf, sizeof(char), out.len, stdout);
            strbuf_setlen(&out, 0);
        }
        else {
            delta_index_splash(&out, nodea->idx->filename, NULL);
	    print_object_insertions(&cache, nodea->idx);
        }
        nodea = nodea->next;
    }
    nodea = b;

    /* for remaining indices */
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
	        print_object_insertions(&cache, nodea->idx);
        }
	    nodea = nodea->next;
        strbuf_setlen(&out, 0);
    }
    strbuf_release(&cache.cache);
    strbuf_release(&out);
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
    if (strbuf_delta_enhanced(&out, &result, a, b))
        fprintf(stdout, "%s", out.buf);
    strbuf_release(&out);
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
    if ((a = find_commit_hash_compat(cl, commit1_sha, strlen(commit1_sha)))
            == NULL) {
        a = find_commit_tag(cl, commit1_sha);
        if (!a) {
            fatal("commit <");
            printf("%s", commit1_sha);
            fprintf(stdout, "> doesn't exists.\n");
            exit(-1);
        }
    }
    if ((b = find_commit_hash_compat(cl, commit2_sha, strlen(commit2_sha)))
            == NULL) {
        b = find_commit_tag(cl, commit1_sha);
        if (!b) {
            fatal("commit <");
            printf("%s", commit1_sha);
            fprintf(stdout, "> doesn't exists.\n");
            exit(-1);
        }
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
    char real_sha1[SHA_STR_SIZE];
    size_t len = SHA_STR_SIZE;

    make_commit_list(&cl);
    if (!noconv) {
        len = char_to_sha1(real_sha1, commit1_hash);
    } else {
        strcpy(real_sha1, commit1_hash);
        len = strlen(commit1_hash);
    }

    if ((a = find_commit_hash_compat(cl, real_sha1, len)) == NULL) {
        a = find_commit_tag(cl, real_sha1);
        if (!a) {
            fatal("commit <");
            print_hash_size(real_sha1, len, stderr);
            fprintf(stdout, "> doesn't exists.\n");
            exit(-1);
        }
    }
    b = get_head_commit(cl);
    do_commit_delta(a, b, minimal);
}

static struct {
    struct delta_stat ds;
    struct index_list *il;
    bool minimal;
    bool set;
    bool original_diff;
    struct pack_file_cache cache;
    struct delta_options *opts;
} directory_delta_var;

void detect_display_crlf(struct strbuf *sb)
{
    for (int i = 0; i < sb->len; i++) {
        if (sb->buf[i] == '\r') {
            printf("\r detected.\n");
            return;
        }
    }
}

int check_entry(const char *path)
{
    struct strbuf a = STRBUF_INIT, b = STRBUF_INIT;
    struct index *idx;
    struct basic_delta_result result;
    struct filespec fs;
    int ret;
    struct strbuf out = STRBUF_INIT;
    size_t res = 0;
    char cmd[1024];
    FILE *f;
    struct strbuf line = STRBUF_INIT;

    if (d_opts.verbose)
        printf("%s\n", path);
    idx = find_file_index_list(directory_delta_var.il, path);
    if (!idx) {
        /* an untracked file */
        return 0;
    }

    get_file_content(&directory_delta_var.cache, &a, idx);
    detect_display_crlf(&a);

    if (d_opts.original_diff) {
        strbuf_init(&line, 1024);
        sprintf(cmd, "diff -u --minimal - %s", idx->filename);
        f = popen(cmd, "w");
        if (!f) {
            die("can't use '%s'", cmd);
        }
        fwrite(a.buf, a.len, 1, f);
        pclose(f);
        return 0;
    }

    ret = filespec_init(&fs, path, "r");
    if (ret < 0) {
        if (errno == ENOENT) {
            res = count_lines(&a);
            directory_delta_var.ds.deletions += res;
            if (directory_delta_var.minimal) {
                fprintf(stdout, "%s: deleted,  " SIZE_T_FORMAT "  deletions\n", path, res);
            } else {
                print_deletion_lines(&a);
            }
            strbuf_release(&a);
            return 0;
        } else {
            die("%s: %s\n", path, strerror(errno));
        }
    }

    filespec_read_safe(&fs, &b);
    detect_display_crlf(&a);
    basic_delta_result_init(&result, NULL);

    if (directory_delta_var.minimal) {
        strbuf_delta_minimal(NULL, &result, &a, &b);
    } else {
        strbuf_delta_enhanced(&out, &result, &a, &b);
    }
    if (result.insertions || result.deletions > 1) {
        fprintf(stdout, "[%s] [%s]\n", path, path);
        directory_delta_var.ds.insertions += result.insertions;
        directory_delta_var.ds.deletions += result.deletions;

        if (directory_delta_var.minimal) {
            print_delta_stat(&result);
        } else {
            fprintf(stdout, "%s\n", out.buf);
            strbuf_release(&out);
        }
    }
    basic_delta_result_release(&result);
    strbuf_release(&a);
    strbuf_release(&b);
    filespec_free(&fs);
    return 0;
}

void prepare_file_delta(bool minimal)
{
    struct commit_list *cl;

    if (directory_delta_var.set)
        return;

    make_commit_list(&cl);
    if (!cl) die("Nothing checked in.\n");
    directory_delta_var.ds.insertions = 0;
    directory_delta_var.ds.deletions = 0;
    directory_delta_var.minimal = minimal;
    directory_delta_var.il = get_head_commit_list(cl);
    directory_delta_var.cache.pack_file_path = PACK_FILE;
    directory_delta_var.opts = NULL;
    directory_delta_var.original_diff = d_opts.original_diff;
    strbuf_init(&directory_delta_var.cache.cache, 0);
    cache_pack_file(&directory_delta_var.cache);
    directory_delta_var.set = true;

    commit_list_del(&cl);
}

/*
 * compares the single file specified by its path with its HEAD's copy.
 */
void do_single_file_delta(const char *path, bool minimal)
{
    prepare_file_delta(minimal);
    check_entry(path);
    if (d_opts.summary) {
        size_t insertions = directory_delta_var.ds.insertions;
        size_t deletions = directory_delta_var.ds.deletions;

        if (insertions || deletions) {
            fprintf(stdout, "Summary: \n");
        }
        if (insertions) {
            fprintf(stdout, SIZE_T_FORMAT "  %s", insertions, insertions == 1 ?
                "insertion" : "insertions");
        }
        if (insertions && deletions) {
            printf(", ");
        }
        if (deletions) {
            fprintf(stdout,  SIZE_T_FORMAT " %s", deletions, deletions == 1 ?
                "deletion" : "deletions");
        }
        putchar('\n');
    }
    invalidate_cache(&directory_delta_var.cache);
}

void do_directory_delta(const char *dir, bool minimal, struct delta_options *opts)
{
    prepare_file_delta(minimal);
    if (for_each_file_in_directory_recurse(dir, check_entry) == -1) {
        die("error occurred while reading directory %s", dir);
    }

    if (opts->summary) {
        size_t insertions = directory_delta_var.ds.insertions;
        size_t deletions = directory_delta_var.ds.deletions;

        if (insertions || deletions) {
            fprintf(stdout, "Summary: \n");
        }
        if (insertions) {
            fprintf(stdout,  SIZE_T_FORMAT " %s", insertions, insertions == 1 ?
                "insertion" : "insertions");
        }
        if (insertions && deletions) {
            printf(", ");
        }
        if (deletions) {
            fprintf(stdout,  SIZE_T_FORMAT " %s", deletions, deletions == 1 ?
                "deletion" : "deletions");
        }
        putchar('\n');
    }
    invalidate_cache(&directory_delta_var.cache);
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

void delta_parse_single_option(struct delta_options *opts, int *i,
                               char *argv[])
{
    struct stat st;
    struct strbuf path = STRBUF_INIT;
    int count = *i;

    if (is("--help") || is("-h"))
        opts->help = true;
    else if (is("-m") || is("--minimal"))
        opts->minimal = true;
    else if (is("--hash"))
        opts->commit = true;
    else if (is("--file") || is("-f"))
        opts->file = true;
    else if (is("--diff") || is("-od"))
        opts->original_diff = true;
    else if (is("--verbose") || is("-v"))
        opts->verbose = true;
    else if (is("--summary") || is ("-s"))
        opts->summary = true;
    else if (is("--debug") || is ("-d"))
        opts->debug = true;
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
        opts->hash_arg1 ? (opts->hash_arg2 = argv[count])
                        : (opts->hash_arg1 = argv[count]);
    } else if (!opts->commit || !opts->file) {
        if (stat(argv[count], &st) < 0) {
            opts->commit = true;
            opts->hash_arg1 ? (opts->hash_arg2 = argv[count])
                            : (opts->hash_arg1= argv[count]);
            opts->guessed = true;
            return;
        }
        if (S_ISDIR(st.st_mode)) {
            opts->recursive = true;
            opts->file = true;
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

        do_directory_delta(".", 0, opts);
        exit(0);
    }
    for (int i = 1; i < argc; i++) {
        delta_parse_single_option(opts, &i, argv);
    }
}

int delta_main(int argc, char *argv[])
{
    directory_delta_var.set = false;
    delta_parse_options(&d_opts, argc, argv);
    if (d_opts.help) {
        printf("Usage: peg compare (path)\n");
        printf("\n");
        printf("options: \n");
        printf("\t(-h | --help) | (--file)\n");
        exit(-1);
    }
    if (d_opts.commit && !d_opts.file) {
        if (d_opts.hash_arg1 && d_opts.hash_arg2) {
            commit_delta(d_opts.hash_arg1, d_opts.hash_arg2, d_opts.minimal);
        } else if (d_opts.hash_arg1) {
            do_single_commit_delta(d_opts.hash_arg1, d_opts.minimal, 1);
        } else if (d_opts.hash_arg2) {
            do_single_commit_delta(d_opts.hash_arg2, d_opts.minimal, 1);
        } else if (d_opts.guessed) {
            die("no sha1 or path specified.\n");
        } else {
            die("please provide atleast one argument for sha1.\n");
        }
    } else if (!d_opts.commit && d_opts.file) {
        if (d_opts.recursive) {
            do_directory_delta(d_opts.file_name, d_opts.minimal, &d_opts);
        } else if (!d_opts.file_name && d_opts.guessed) {
            die("no sha1 or path specified.\n");
        } else if (d_opts.guessed || d_opts.file_name) {
            do_single_file_delta(d_opts.file_name, d_opts.minimal);
        } else {
            die("no path specified.\n");
        }
    } else {
        do_directory_delta(".", d_opts.minimal, &d_opts);
    }
    return 0;
}
