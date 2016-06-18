#include "status.h"
#include "commit.h"
#include "util.h"
#include <dirent.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define CHECKED (0x1 << 5)
#define mark_checked(x) (x->flags |= CHECKED)
#define is_checked(x) (x->flags & CHECKED)

struct status_options {
    bool new;
    bool modified;
    bool old;
} status_opts;

void print_status_html(struct node *root);

char *file_path(char *parent_dir, char *name)
{
    struct strbuf buffer;
    strbuf_init(&buffer, 0);
    strbuf_addstr(&buffer, parent_dir);
    strbuf_addch(&buffer, '/');
    strbuf_addstr(&buffer, name);
    return (buffer.buf);
}

/*
 * To implement stack by linked list of struct d_node s pionters to
 * current node , head of linked list and previous node are needed
 */
struct d_node *d_root = NULL, *d_sptr = NULL, *d_ptr = NULL;
int count_new = 0, count_modified = 0, count_cached = 0, count_deleted = 0;
struct node *root = NULL;

int status(char *p)
{
    if (root) {
        root = NULL;
    }
    DIR *directory;   //  create  a stream to the directory , opendir() returns
                      //  object of DIR type
    char *name;       // to store name of the file
    struct dirent *d; // dirent stands for directory entry
    struct stat status;
    struct strbuf buffer1, buffer2;
    char c;
    struct node *ptr = NULL, *sptr = NULL;
    int response, file_exists_db;
    struct d_node *d_tptr = NULL;
    char *parent_dir = p;
    struct commit_list *cl;
    struct index_list *il;
    struct index *idx;
    struct pack_file_cache cache = PACK_FILE_CACHE_INIT;

    make_commit_list(&cl);
    il = get_head_commit_list(cl);
    cache_pack_file(&cache);
back:
    directory = opendir(parent_dir); // open the current working directory
    if (directory == NULL)
        die("Could not open the directory %s, %s\n", parent_dir,
            strerror(errno));

    while ((d = readdir(directory))) // start reading the content of directory
                                     // which can be a file or a directory
    {

        name = d->d_name;
        if (!strcmp(name, ".") || !strcmp(name, "..") || !strcmp(name, PEG_DIR))
            continue;
        name = file_path(parent_dir, name);
        if (stat(name, &status) < 0)
            die("can't stat %s, %s", name, strerror(errno));

        /* if the object returned by readdir is a regular file then
         * we will create a FILE stream to the file and read the contents of the
         * file
         * if the stream is not created due to any reason it will be reported
         * if the stream is created we will store the content of the file in
         * string buffer1
         * now we will retrieve a file named same as the name of current file
         * and store it in string buffer2
         * we will compare these files using the function strcmp()
         * if the files are same then it means that there is no modification
         * if there is no file by that name then it will mean that the file is
         * newly created
         * if the file by that name exists but contents are different then it
         * will mean modification*/

        if (S_ISREG(status.st_mode)) {

            idx = find_file_index_list(il, name);
            if (idx) {
                file_exists_db = 1;
                mark_checked(idx);
            } else {
                file_exists_db = 0;
            }
            // file_exists_db = find_file_from_head_commit(
            //     name, &buffer2); // reading file named name(means same as the
            //                      // name of current file) from database and
            //                      // storing it in string buffer2
            if (file_exists_db) {
                response = (idx->st.st_mtime < status.st_mtime);
                if (response) {
                    int fd = open(name, O_RDONLY);
                    struct strbuf buf1 = STRBUF_INIT, buf2 = STRBUF_INIT;
                    if (fd < 0) {
                        die("can't open %s, %s", name, strerror(errno));
                    }

                    strbuf_read(&buf1, fd, status.st_size);
                    if (close(fd) < 0)
                        die("can't close %s, %s", name, strerror(errno));
                    get_file_content(&cache, &buf2, idx);
                    response = strbuf_cmp(&buf1, &buf2);
                    strbuf_release(&buf1);
                    strbuf_release(&buf2);
                }
            }
            // data type  of buffer1 and 2  is strbuf

            /* Now we need to make a linked list of all the file names and their
             * status
             * where by status  we mean
             * 1 for modified
             * 2 for new
             * 3 for not yet commited
             * 4 for no modification
            */
            if (file_exists_db) {
                if (response == 0) {
                    ptr = createnode();
                    intialise_node(&ptr, name, 0, NULL);
                    insert_node(&root, &ptr, &sptr);
                } else {
                    count_modified++;
                    ptr = createnode();
                    intialise_node(&ptr, name, S_MODIFIED, NULL);
                    insert_node(&root, &ptr, &sptr);
                }
            } else {
                count_new++;
                ptr = createnode();
                intialise_node(&ptr, name, S_NEW, NULL);
                insert_node(&root, &ptr, &sptr);
            }
        }
        /*
        * if the current object being pointed to by dirent is a directory then
        * we will store this pointer in a stack ,so that its file can be
        * compared
        * when we are done comparing the files in the current directory */
        else {
            if (S_ISDIR(status.st_mode)) {
                if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
                    continue;
                push(d, parent_dir);
            }
        }
    }
    closedir(directory);

    if (d_sptr != d_root) {
        d_tptr = pop();
        parent_dir = path(d_tptr);
        goto back;
    }

    /*
     * deleted files
     */
    {
        struct index_list *node = il;

        while (node) {
            if (node->idx->flags == DELETED) {
                node = node->next;
                continue;
            }
            if (is_checked(node->idx)) {
                node = node->next;
            } else {
                ptr = createnode();
                intialise_node(&ptr, node->idx->filename, S_DELETED, NULL);
                insert_node(&root, &ptr, &sptr);
                node = node->next;
                count_deleted++;
            }
        }
    }

    commit_list_del(&cl);
    return 0;
}

int status_main(
    int argc,
    char *argv[]) // arguments in main send to give options to the user
{
    if (argc > 1) {
        if (!strcmp(argv[1], "--html")) {
            status(".");
            print_status_html(root);
            return 0;
        } else
            status(argv[1]);
    } else {
        status(".");
    }
    print_status(root);
    return 0;
}

struct node *createnode()
{
    return ((struct node *)malloc(sizeof(struct node)));
}

void intialise_node(struct node **node, char *name, int status,
                    struct node *next)
{
    (*node)->name = name;
    (*node)->status = status;
    (*node)->next = NULL;
}

void insert_node(struct node **root, struct node **current_node,
                 struct node **previous_node)
{
    if (*root == NULL) {
        *root = *current_node;
        *previous_node = *current_node;
    } else {
        (*previous_node)->next = *current_node;
        *previous_node = *current_node;
    }
}

void push(struct dirent *d, char *parent_dir)
{
    d_ptr = ((struct d_node *)malloc(sizeof(struct d_node)));
    strbuf_init(&d_ptr->name, 0);
    strbuf_addstr(&d_ptr->name, d->d_name);
    d_ptr->parent_dir = parent_dir;
    d_ptr->next = NULL;

    if (d_root == NULL) {
        d_root = d_ptr;
        d_sptr = d_ptr;
        d_ptr->previous = NULL;
    } else {
        d_sptr->next = d_ptr;
        d_ptr->previous = d_sptr;
        d_sptr = d_ptr;
    }
}

struct d_node *pop()
{
    struct d_node *tptr;
    tptr = d_sptr;
    d_sptr = d_sptr->previous;
    d_sptr->next = NULL;
    return tptr;
}

char *path(struct d_node *tptr)
{
    struct strbuf parent_dir;
    strbuf_init(&parent_dir, 0);
    strbuf_addstr(&parent_dir, tptr->parent_dir);
    strbuf_addch(&parent_dir, '/');
    strbuf_addstr(&parent_dir, tptr->name.buf);
    return (parent_dir.buf);
}

void print_status(struct node *root)
{
    struct node *tptr = NULL;
    tptr = root;
    if (!count_modified && !count_new && !count_deleted) {
        printf("Working directory clean, nothing changed.\n");
        return;
    }
    if (count_modified || count_deleted) {
        printf("Changes not staged for commit, please use " YELLOW
               "\"peg insert --all --modified\"" RESET " to add the"
               "modified files\n");
        while (tptr != NULL) {
            if (tptr->status == S_MODIFIED || tptr->status == S_DELETED) {
                printf(RED);
                printf("    %s: %s\n",
                       tptr->status == S_MODIFIED ? "modified" : "deleted",
                       tptr->name);
            }
            tptr = tptr->next;
        }
        printf(RESET);
    }
    tptr = root;
    if (count_new) {
        printf("Untracked files exist in project directory\n      please use "
               "'" YELLOW "peg insert <files>..." RESET "'\n");

        while (tptr != NULL) {
            if (tptr->status == S_NEW) {
                printf(CYAN);
                printf("\t%s\n", tptr->name);
            }
            tptr = tptr->next;
        }
    }
    tptr = root;
    if (status_opts.old) {
        // printf("Changes cached but not commited \n please use '"YELLOW"peg
        // commmit -m <...>"RESET"'");
        while (tptr != NULL) {
            if (tptr->status == 1) {
                printf(BOLD_GREEN);
                printf("\told :  %s\n", tptr->name);
            }

            tptr = tptr->next;
        }
    }
    printf(RESET);
}

void print_status_html(struct node *root)
{
    printf("<html><head><style> li#modified { color: red; } li#new { color: "
           "blue } </style>");
    struct node *tptr = NULL;
    tptr = root;
    printf("<strong>status   file</strong>");
    if (!count_modified && !count_new && !count_deleted) {
        printf("Working directory clean, nothing changed.\n");
        return;
    }
    if (count_modified || count_deleted) {
        printf("<ul color=\"red\">");
        while (tptr != NULL) {
            if (tptr->status == S_MODIFIED || tptr->status == S_DELETED) {
                printf("<li>%s: %s\n</li>",
                       tptr->status == S_MODIFIED ? "modified" : "deleted",
                       tptr->name);
            }
            tptr = tptr->next;
        }
        printf("</ul>\n");
    }

    tptr = root;
    if (count_new) {
        printf("<ul bgcolor=\"#FF9494\" color=white>");
        while (tptr != NULL) {
            if (tptr->status == S_NEW) {
                printf("<li id=new>new: %s</li>\n", tptr->name);
            }
            tptr = tptr->next;
        }
        printf("<ul>\n");
    }
    tptr = root;
    if (status_opts.old) {
        // printf("Changes cached but not commited \n please use '"YELLOW"peg
        // commmit -m <...>"RESET"'");
        while (tptr != NULL) {
            if (tptr->status == 4) {
                printf(BOLD_GREEN);
                printf("\told :  %s\n", tptr->name);
            }

            tptr = tptr->next;
        }
    }
}
