#include "commit.h"
#include "global.h"
#include "sha1-inl.h"

#define IF_TAG(tag) (tag & 1)

struct commit_options {
    struct strbuf msg;
    struct strbuf desc;
    size_t insertions;
    size_t deletions;
    size_t file_modified;
    size_t new_files;
} cm_opts;

void commit_options_init()
{
    strbuf_init(&cm_opts.msg, 0);
    strbuf_init(&cm_opts.desc, 0);
    cm_opts.insertions = 0;
    cm_opts.deletions = 0;
    cm_opts.file_modified = 0;
    cm_opts.new_files = 0;
}

void print_commit_stats(struct commit_options *opts, struct commit *cm)
{
    printf("[ "YELLOW);
    print_hash_size(cm->sha1, 3, stdout);
    if (IF_TAG(cm->flags))
        printf(RESET" ("YELLOW"%s"RESET")", cm->tag);
    else
        printf(RESET " ("DIM"empty"RESET")");
    printf(RESET" ]   %s\n", cm->cmt_msg.buf);

    if (cm->cmt_desc.len) {
        printf("\n%s\n\n", cm->cmt_desc.buf);
    }
#ifdef _WIN32
    printf(YELLOW" %llu"RESET" %s changed, ", opts->file_modified
        + opts->new_files,
        (opts->file_modified + opts->new_files) > 1 ? "files" : "file");

    if (opts->insertions)
        printf(BOLD_GREEN"%llu"RESET" %s(+)", opts->insertions, opts->insertions
            > 1 ? "additions" : "addition");
    if (opts->insertions && opts->deletions)
        printf(", ");
    if (opts->deletions)
        printf(BOLD_RED"%llu"RESET" %s(-)", opts->deletions, opts->deletions > 1
            ? "deletions" : "deletion");
    putchar('\n');
#else
    printf(YELLOW" %zu"RESET" %s changed, ", opts->file_modified
        + opts->new_files,
        (opts->file_modified + opts->new_files) > 1 ? "files" : "file");

    if (opts->insertions)
        printf(BOLD_GREEN"%zu"RESET" %s(+)", opts->insertions, opts->insertions
            > 1 ? "additions" : "addition");
    if (opts->insertions && opts->deletions)
        printf(", ");
    if (opts->deletions)
        printf(BOLD_RED"%zu"RESET" %s(-)", opts->deletions, opts->deletions > 1
            ? "deletions" : "deletion");
    putchar('\n');
#endif
}

void commit_init(struct commit *cm)
{
    time_stamp_init(&cm->stamp);
    cm->flags = 0;
    cm->tag[0] = '\0';
    cm->auth = NULL;
    strbuf_init(&cm->cmt_msg, 0);
    strbuf_init(&cm->cmt_desc, 0);
}

int make_commit_object(struct commit *cm, struct author *auth,
    struct strbuf *head, struct strbuf *desc)
{
    cm->auth = auth;
    strbuf_addbuf(&cm->cmt_msg, head);
    strbuf_addbuf(&cm->cmt_desc, desc);
    return 0;
}

int read_commit_object(struct commit *cm, FILE *f)
{
    size_t len;

    fread(&cm->flags, sizeof(cm->flags), 1, f);
    if (fread(cm->sha1, sizeof(char), HASH_SIZE, f) < HASH_SIZE) {
        die(" error occurred while reading commit file\n");
    }
    if (IF_TAG(cm->flags)) {
        fread(cm->tag, sizeof(char), TAG_SIZE, f);
    }
    cm->auth = MALLOC(struct author, 1);
    author_init(cm->auth);
    author_read(cm->auth, f);
    fread(&len, sizeof(size_t), 1, f);
    strbuf_fread(&cm->cmt_msg, len, f);
    fread(&len, sizeof(size_t), 1, f);
    strbuf_fread(&cm->cmt_desc, len, f);
    time_stamp_read(&cm->stamp, f);
    fread(&cm->commit_index, sizeof(size_t), 1, f);
    fread(&cm->commit_length, sizeof(size_t), 1, f);
    for (int i = 0; i < HASH_SIZE; i++) {
        sprintf(cm->sha1str + 2 * i, "%02x", (uint8_t)cm->sha1[i]);
    }
    cm->sha1str[SHA_STR_SIZE] = '\0';
    return 0;
}

int write_commit_object(struct commit *cm, FILE *f)
{
    fwrite(&cm->flags, sizeof(cm->flags), 1, f);
    fwrite(cm->sha1, sizeof(char), HASH_SIZE, f);
    if (IF_TAG(cm->flags)) {
        fwrite(cm->tag, sizeof(char), TAG_SIZE, f);
    }
    author_write(cm->auth, f);
    fwrite(&cm->cmt_msg.len, sizeof(size_t), 1, f);
    fwrite(cm->cmt_msg.buf, sizeof(char), cm->cmt_msg.len, f);
    fwrite(&cm->cmt_desc.len, sizeof(size_t), 1, f);
    fwrite(cm->cmt_desc.buf, sizeof(char), cm->cmt_desc.len, f);
    write_time_stamp(&cm->stamp, f);
    fwrite(&cm->commit_index, sizeof(size_t), 1, f);
    fwrite(&cm->commit_length, sizeof(size_t), 1, f);
    return 0;
}

void commit_list_init(struct commit_list *head)
{
    head->item = NULL;
    head->next = NULL;
    head->count = 0;
}

void commit_list_add(struct commit_list **last, struct commit_list *node)
{
    (*last)->next = node;
    (*last) = node;
}

void commit_del(struct commit *cm)
{
    author_del(cm->auth);
    strbuf_release(&cm->cmt_msg);
    strbuf_release(&cm->cmt_desc);
}

void commit_list_del(struct commit_list **head)
{
    struct commit_list *node = *head;
    struct commit_list *del = node;

    while (node) {
        del = node;
        node = node->next;
        commit_del(del->item);
        free(del->item);
        free(del);
    }
}

size_t make_commit_list(struct commit_list **head)
{
    size_t count = 0;
    struct commit_list *node = NULL, *new = NULL, *last = NULL;
    struct commit *cm = NULL;
    FILE *f = fopen(COMMIT_INDEX_FILE, "rb");

    if (!f) die("unable to open %s\n, %s", COMMIT_INDEX_FILE, strerror(errno));
    fread(&count, sizeof(size_t), 1, f);
    *head = NULL;
    while (count--) {
        cm = MALLOC(struct commit, 1);
        node = MALLOC(struct commit_list, 1);
        commit_init(cm);
        commit_list_init(node);
        node->item = cm;
        read_commit_object(cm, f);
        if (*head) {
            commit_list_add(&last, node);
            (*head)->count++;
        }
        else {
            *head = node;
            last = node;
            (*head)->count = 1;
        }
    }

    fclose(f);
    return 0;
}

struct commit *find_commit_hash(struct commit_list *cl,
    char sha1[SHA_STR_SIZE])
{
    struct commit_list *node = cl;

    while (node) {
        if (!strcmp((const char*)node->item->sha1str, (const char*)sha1))
            return node->item;
        node = node->next;
    }
    return NULL;
}

struct commit *find_commit_generic(struct commit_list *cl, commit_match_fn *fn)
{
    struct commit_list *node = cl;

    while (node) {
        if (fn(node->item))
            return node->item;
        node = node->next;
    }
    return NULL;
}

size_t for_each_commit(struct commit_list *head, commit_fn *callback)
{
    ssize_t count = 0;
    struct commit_list *node = head;

    while (node) {
        if (!callback(node->item))
            count++;
        node = node->next;
    }
    return count;
}

void flush_commit_list(struct commit_list *head)
{
    FILE *commit_file = fopen(COMMIT_INDEX_FILE, "wb");
    struct commit_list *node = head;

    if (!(commit_file))
        die("Unable to open commit file. %s\n", strerror(errno));
    fwrite(&head->count, sizeof(size_t), 1, commit_file);
    while (node) {
        write_commit_object(node->item, commit_file);
        node = node->next;
    }
    fclose(commit_file);
    commit_list_del(&head);
}

void insert_index_list(struct index_list **last, struct index_list *node)
{
    (*last)->next = node;
    (*last) = node;
}

void make_index_list_from_commit(struct commit *node, struct index_list **head)
{
    struct index_file_cache cache = { FILE_INDEX_FILE, STRBUF_INIT };
    struct index *new = NULL;
    struct index_list *last = NULL, *n = NULL;
    size_t offset = node->commit_index;
    size_t count = 0;

    *head = NULL;
    if (!node) return;
    cache_index_file(&cache);
    while (offset != (node->commit_index + node->commit_length)) {
        new = make_index_from_cache(&cache, offset);
        n = MALLOC(struct index_list, 1);
        n->idx = new;
        n->next = NULL;
        offset += sizeof(struct index);
        count++;

        if (*head) insert_index_list(&last, n);
        else {
            *head = n;
            last = n;
        }
    }
    strbuf_release(&cache.cache);
}

struct commit *find_commit_hash_compat(struct commit_list *cl,
    char *sha1, size_t len)
{
    struct commit_list *node;
    struct commit_list *result = NULL, *last = NULL;

    node = cl;
    while (node) {
        if (hash_starts_with(node->item->sha1str, sha1, len)) {
            return node->item;
        }
        node = node->next;
    }
    return NULL;
}

struct index *find_file_index_list(struct index_list *head, const char *file)
{
    struct index_list *node = head;

    while (node) {
        if (!strcmp((const char*)node->idx->filename, file)) {
            return node->idx;
        }
        node = node->next;
    }
    return NULL;
}

struct index_list *get_last_node(struct index_list *head)
{
    struct index_list *node, *prev;
    node = head;
    prev = head;
    while (node) {
        prev = node;
        node = node->next;
    }
    return prev;
}

struct index_list *get_head_commit_list(struct commit_list *head)
{
    struct commit_list *node, *prev;
    struct index_list *ret = NULL;
    node = head;
    prev = head;

    while (node) {
        prev = node;
        node = node->next;
    }
    if (prev)  make_index_list_from_commit(prev->item, &ret);
    return ret;
}

bool find_file_from_head_commit(const char *name, struct strbuf *buf)
{
    struct commit_list *head;
    struct pack_file_cache cache = PACK_FILE_CACHE_INIT;
    make_commit_list(&head);
    struct index_list *last = get_head_commit_list(head);
    struct index *idx = find_file_index_list(last, name);
    if (!idx) return false;
    cache_pack_file(&cache);
    strbuf_add(buf, cache.cache.buf + idx->pack_start, idx->pack_len);
    strbuf_release(&cache.cache);
    return true;
}

struct index_list *copy_index_list(struct index_list *src)
{
    struct index_list *node = src, *temp = NULL, *new = NULL, *last;
    while (node) {
        temp = MALLOC(struct index_list, 1);
        temp->idx = make_new_index(node->idx->filename, node->idx->pack_start,
            node->idx->pack_len, node->idx->flags);
        temp->next = NULL;

        if (new)
            insert_index_list(&last, temp);
        else {
            new = temp;
            last = temp;
        }
        node = node->next;
    }
    return new;
}

void write_index_list(struct index_list *head, struct strbuf *cache)
{
    struct index_list *node = head;
    while (node) {
        strbuf_add(cache, (void*)node->idx, sizeof(struct index));
        node = node->next;
    }
}

void transfer_staged_data(struct cache_object *co, struct index_list **head,
    char *sha1)
{
    struct cache_index_entry_list *node = co->ci.entries;
    struct index *exist;
    struct pack_file_cache cache = PACK_FILE_CACHE_INIT;
    struct strbuf temp = STRBUF_INIT;
    struct strbuf comp = STRBUF_INIT;
    struct strbuf old = STRBUF_INIT;
    size_t idx;
    struct basic_delta_result result;
    struct index_list *last = NULL, *new;
    peg_sha_ctx ctx;

    sha1_init(&ctx);
    cache_pack_file(&cache);
    // last can be NULL also in case head is NULL;
    last = get_last_node(*head);
    idx = cache.cache.len;
    while (node) {
        exist = find_file_index_list(*head, node->file_path.buf);
        basic_delta_result_init(&result, NULL);
        if (!exist) {
            // we're handling new file
            exist = make_new_index(node->file_path.buf,
                     idx, node->len, 0);
            // write to pack file
            strbuf_add(&temp, co->cc.cache_buf.buf + node->start, node->len);
            cm_opts.insertions += count_lines(&temp);
            strbuf_addbuf(&cache.cache, &temp);
            sha1_update(&ctx, temp.buf, temp.len);
            strbuf_release(&temp);
            idx = cache.cache.len;
            new = MALLOC(struct index_list, 1);
            new->idx = exist;
            new->next = NULL;
            cm_opts.new_files++;
            if (last) insert_index_list(&last, new);
            else {
                *head = new;
                last = new;
            }
            node = node->next;
            continue;
        }
        strbuf_add(&old, cache.cache.buf + exist->pack_start, exist->pack_len);
        exist->pack_start = idx;
        exist->pack_len = node->len;
        exist->flags |= 0x80;
        strbuf_add(&temp, co->cc.cache_buf.buf + node->start, node->len);
        strbuf_delta_minimal(NULL, &result, &old, &temp);
        cm_opts.insertions += result.insertions;
        cm_opts.deletions += result.deletions;
        cm_opts.file_modified++;
        strbuf_addbuf(&cache.cache, &temp);
        sha1_update(&ctx, temp.buf, temp.len);
        strbuf_release(&temp);
        strbuf_release(&old);
        idx = cache.cache.len;
        node = node->next;
    }
    sha1_final((unsigned char*)sha1, &ctx);
    flush_pack_cache(&cache);
    strbuf_release(&cache.cache);
}

void set_tag(const char *sha1, size_t len, const char *tag)
{
    struct commit_list *cl, *node;
    struct commit *cm;

    make_commit_list(&cl);

    if (!cl)
        die("There is no "BLACK"commit"RESET" to tag '"YELLOW"%s"RESET"'.\n",
            tag);

    if (!sha1) {
        /* if no sha1 is provided then we'll continue with HEAD commit*/
        struct commit_list *prev = cl;

        node = cl;
        while (node) {
            prev = node;
            node = node->next;
        }
        cm = prev->item;
    } else {
        cm = find_commit_hash_compat(cl, (char*)sha1, len);
        if (!cm)
            die("we found no such commit '"YELLOW"%s"RESET"'.\n", sha1);
    }
    if (len >= TAG_SIZE)
        die("good tags don't have such huge name like '"YELLOW"%s"RESET"'.\n"
            "        (Try keeping small and simple tags.)\n",
            tag);
    strcpy(cm->tag, tag);
    cm->flags = TAG_FLAG;
    /* print some information */
    printf("[ "YELLOW);
    print_hash_size(cm->sha1, 3, stdout);
    printf(RESET" ("YELLOW"%s"RESET")", cm->tag);
    printf(RESET" ]   %s\n", cm->cmt_msg.buf);
    flush_commit_list(cl);
}

int log_commit(struct commit *cm)
{
    printf("[ "YELLOW);
    print_hash_size(cm->sha1, 3, stdout);
    printf(RESET" ("YELLOW"%s"RESET")", cm->tag);
    printf(RESET" ]");
    fprintf(stdout, "  %s\n\n", cm->cmt_msg.buf);
    if (cm->cmt_desc.len)
        fprintf(stdout, "\n\t%s\n\n", cm->cmt_desc.buf);
    cm->stamp._tm = localtime(&cm->stamp._time);
    fprintf(stdout, "Author:  %s <%s>\n", cm->auth->name.buf,
        cm->auth->email.buf);
    fprintf(stdout, "Date:    %s\n", asctime(cm->stamp._tm));
    return 0;
}

void list_tags()
{
    struct commit_list *cl;
    struct commit_list *node;

    make_commit_list(&cl);
    if (!cl) return;

    node = cl;
    while (node) {
        if (IF_TAG(node->item->flags))
            fprintf(stdout, YELLOW"%s\n"RESET, node->item->tag);
        node = node->next;
    }
    commit_list_del(&cl);
}

void print_commits()
{
    struct commit_list *cm;
    make_commit_list(&cm);
    for_each_commit(cm, log_commit);
    commit_list_del(&cm);
}

void finalize_commit(struct commit_list *head, struct commit *new)
{
    struct commit_list *newnode = MALLOC(struct commit_list, 1), *node = head;
    struct commit_list *last = head;

    newnode->count = 0;
    newnode->item = new;
    newnode->next = NULL;
    if (!head) {
        head = newnode;
        head->count = 1;
    }
    else {
        while (node) {
            last = node;
            node = node->next;
        }
        commit_list_add(&last, newnode);
        head->count += 1;
    }
    print_commit_stats(&cm_opts, new);
    flush_commit_list(head);
}

int generate_new_commit(struct strbuf *cmt, struct strbuf *det,
    char tag[TAG_SIZE], int16_t flags)
{
    struct commit_list *head = NULL;
    struct cache_object co;
    make_commit_list(&head);
    struct commit *new = MALLOC(struct commit, 1);
    struct index_list *head_cm = get_head_commit_list(head);
    struct index_list *copy = copy_index_list(head_cm);
    size_t index, start, len;
    struct author *a = MALLOC(struct author, 1);
    struct index_file_cache cache = INDEX_CACHE_INIT;

    author_init(a);
    cache_object_init(&co);
    if (!co.ci.entries)
        die("stage data empty\n");

    commit_init(new);
    time_stamp_init(&new->stamp);
    cache_index_file(&cache);
    get_global_author(a);
    new->auth = a;
    new->flags = flags;
    if (IF_TAG(flags)) {
        strcpy(new->tag, tag);
    }
    transfer_staged_data(&co, &copy, new->sha1);
    strbuf_addbuf(&new->cmt_msg, cmt);
    strbuf_addbuf(&new->cmt_desc, det);
    start = cache.cache.len;
    new->commit_index = start;
    write_index_list(copy, &cache.cache);
    len = cache.cache.len - start;
    new->commit_length = len;
    finalize_commit(head, new);
    flush_index(&cache);

    cache_object_clean(&co);
    strbuf_release(&cache.cache);
    return 0;
}

int commit(int argc, char *argv[])
{
    struct strbuf msg = STRBUF_INIT, desc = STRBUF_INIT;
    char tag[TAG_SIZE];
    int16_t flags = 0;

    commit_options_init();
    if (argc < 2)
        die("empty args. See --help for usage.\n");

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            printf("Usage: "PEG_NAME" commit -m message -d description\n");
            exit(0);
        } else
        if (!strcmp(argv[i], "-m") || !strcmp(argv[i], "--message")) {
            strbuf_addstr(&msg, argv[i + 1]);
            i++;
        } else
        if (!strcmp(argv[i], "-d")) {
            strbuf_addstr(&desc, argv[i + 1]);
            i++;
        } else
        if (!strcmp(argv[i], "--log")) {
            print_commits();
            exit(0);
        } else
        if (!strcmp(argv[i], "-t") || !strcmp(argv[i], "--tag")) {
            if (strlen(argv[i + 1]) > TAG_SIZE)
                die("length of tag should be less than %d\n", TAG_SIZE);
            strcpy(tag, argv[i + 1]);
            flags |= 1;
            i++;
        }

    }
    if (!msg.len)
        die("no message provided\n");
    generate_new_commit(&msg, &desc, tag, flags);
    return 0;
}
