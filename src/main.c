#include "commit.h"
#include "stage.h"
#include "delta.h"
#include "path.h"
#include "project-init.h"

#include <math.h>

enum cmd_type {
    CREATE,
    INSERT,
    COMMIT,
    REVERT,
    RST,
    HIST,
    STATUS,
    COMPARE,
    SEE,
    TAG,
    SET,
    LIST,
    HELP,
    INVALID,
    USER,
    UNKNOWN
};

struct command_type {
    enum cmd_type t;
    char *cmd;
};

static struct command_type cmds[] = {
    { CREATE, "create" },
    { INSERT, "insert" },
    { COMMIT, "commit" },
    { RST, "reset" },
    { HIST, "hist" },
    { STATUS, "status" },
    { REVERT, "revert" },
    { COMPARE, "compare" },
    { LIST, "list" },
    { TAG, "tag" },
    { HELP, "help" },
    { SEE, "seeChanges" },
    { SET, "setAlias" }
};

#define COMMAND_COUNT 13

static int argc;
static char **argv;

struct command {
    struct strbuf name;
    enum cmd_type type;
    int match;
    int argc;
    char **argv;
};

struct command_alias {
    struct strbuf alias;
    struct command *cmd;
};

struct core_commands {
    int count;
    int toexecute;
    struct command cmd;
    struct core_commands *next;
};

/**
 * various commands look like sql commands
 *  1) Command to initialize project
 *        peg create repo [here | path/to/project]
 *  2) Command to reinitialize project
 *        peg create repo [here | path/to/project]
 *  3) Command to add a file
 *        peg insert [files | directories | all]
 *         [files => file | files]
 *         [directories => directory | directories]
 *      Note: only changed files are added to stage area
 *  4) Command to commit a change
 *        peg commit message="some message" (description="some description" |
 *                            tag="v1.x.x.x")
 *  5) Command to revert a file
 *        peg revert <filename> to nth commit
 *  6) Command to see the changes you have made
 *        peg see changes in <filename>
 *  7) Command to compare two commits
 *        peg compare <commit1> <commit2>
 *  8) Command to compare a particular commit with HEAD commit
 *        peg compare <commit> with latest
 *  9) Command to set aliases
 */

void init_command(struct command *cmd)
{
    strbuf_init(&cmd->name, 0);
    cmd->type = UNKNOWN;
    cmd->argc = 0;
    cmd->argv = NULL;
}

// int similar(char *str1, char *str2)
// {
//     int match = 0;
//     size_t len1 = strlen(str1);
//     size_t len2 = strlen(str2);
//     size_t min = len1 < len2 ? len1 : len2;

//     for (int i = 0; i < )
//     return 0;
// }

// void suggest_commands(const char *name)
// {

// }

enum cmd_type find_command(struct core_commands *cmds, const char *cmd)
{
    struct core_commands *node = cmds;

    while (node) {
        if (!strcmp(node->cmd.name.buf, cmd))
            return node->cmd.type;
        node = node->next;
    }
    return UNKNOWN;
}

void read_user_cmds(struct core_commands **head, struct core_commands *last)
{

}

void gen_core_commands(struct core_commands **head)
{
    struct core_commands *last;
    struct core_commands *new;

    *head = NULL;
    for (int i = 0; i < COMMAND_COUNT; i++) {
        new = malloc(sizeof(struct core_commands));
        init_command(&new->cmd);
        strbuf_addstr(&new->cmd.name, cmds[i].cmd);
        new->cmd.type = cmds[i].t;
        new->toexecute = false;
        new->count = 0;
        new->next = NULL;
        if (!*head) {
            *head = new;
        } else {
            last->next = new;
        }
        last = new;
        (*head)->count++;
    }

    read_user_cmds(head, last);
}

int skip_command_prefix(struct strbuf *args)
{
    int i = 0;
    while (i < args->len && args->buf[i++] != ' ')
        ;
    return i;
}

enum match_type {
    MATCHED,
    UNMATCH,
    END
};

void suggest_create_usage()
{
    printf(" May be you wanted to say 'peg create repo ...'\n");
    printf("    (See 'peg help create' for proper syntax)\n");
}

bool help(int argc, char **argv)
{
    return true;
}

bool parse_set_cmd(int argc, char **argv)
{
    return true;
}

bool create_tag(int argc, char *argv[])
{
    if (argc == 1) {
        list_tags();
        return true;
    }

    if (argc == 2)
        set_tag(NULL, 0, argv[1]);
    else set_tag(argv[1], strlen(argv[1]), argv[2]);
    return true;
}

bool exec_commands_args(enum cmd_type cmd, int count, char **in)
{
    size_t peek;
    switch (cmd) {
        case CREATE:
            return initialize_empty_project(count, in);
        case INSERT:
            return stage_main(count, in);
        case REVERT:
            return checkout(count, in);
        case COMMIT:
            return commit(count, in);
        case HELP:
            return help(count, in);
        case COMPARE:
            return delta_main(count, in);
        case SEE:
            return delta_main(count, in);
        case SET:
            return parse_set_cmd(count, in);

        case RST:
            {
                struct cache_object co;
                cache_object_init(&co);
                cache_object_clean(&co);
                exit(0);
            }

        case STATUS:
            return status_main(count, in);
        case TAG:
            return create_tag(count, in);
        case HIST:
        {
            print_commits();
            break;
        }
        case LIST:
            return list_index(count, in);

        case USER:
        case INVALID:
        case UNKNOWN:
        {}
    }
    return true;
}

void exec_cmd(enum cmd_type cmd, int argc, char **argv)
{
    exec_commands_args(cmd, argc, argv);
}

void join_args(struct strbuf *args, int argc, char *argv[])
{
    struct strbuf temp = STRBUF_INIT;
    int i = 0, len = 0, find;

    for (i = 1; i < argc; i++) {
        strbuf_addstr(&temp, argv[i]);
        len = strbuf_findch(&temp, ' ');
        if (len >= 0) {
            if (i != 1)
                strbuf_addch(args, ' ');
            strbuf_addstr(args, "\"");
            strbuf_addbuf(args, &temp);
            strbuf_addch(args, '"');
        } else {
            if (i != 1)
                strbuf_addch(args, ' ');
            strbuf_addbuf(args, &temp);
        }
        strbuf_setlen(&temp, 0);
    }
}

void print_help()
{
    printf("no help!\n");
}

int main(int argc, char *argv[])
{

    struct strbuf args = STRBUF_INIT;
    struct core_commands *head;
    enum cmd_type t;

    if (argc == 1 || !strcmp(argv[1], "help")) {
        print_help();
        exit(0);
    }
    if (strcmp(argv[1], "create")) {
        struct stat st;
        if (stat(".peg", &st)) {
            die("Not a peg repository.\n");
        }
    }
    if (argc < 2) {
        die("No arguments provided.\n");
    }

    join_args(&args, argc, argv);
    gen_core_commands(&head);
    t = find_command(head, argv[1]);
    if (t == UNKNOWN) {
        fprintf(stderr, "peg: '"RED"%s"RESET"' unknown command.\n", argv[1]);
        //suggest_commands(argv[1]);
        exit(0);
    }
    exec_cmd(t, --argc, (argv + 1));
    return 0;
}
