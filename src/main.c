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
    COMPARE,
    SEE,
    TAG,
    SET,
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
    { REVERT, "revert" },
    { COMPARE, "compare" },
    { TAG, "tag" },
    { HELP, "help" },
    { SEE, "seeChanges" },
    { SET, "setAlias" }
};

#define COMMAND_COUNT 10

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

enum cmd_type find_command(struct core_commands *cmds, struct strbuf *cmd)
{
    struct strbuf prefix = STRBUF_INIT;
    size_t i = 0;
    struct core_commands *node = cmds;

    while (i < cmd->len && cmd->buf[i] != ' ')
        strbuf_addch(&prefix, cmd->buf[i++]);
    /* match the command */

    while (node) {
        if (!strcmp(node->cmd.name.buf, prefix.buf))
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
    if (argc < 3)
        die("arguments should be 'peg tag <commit> <tag>'\n");
    set_tag(argv[1], strlen(argv[1]), argv[2]);
    return true;
}

bool exec_commands_args(enum cmd_type cmd, int out, char **in)
{
    size_t peek;
    switch (cmd) {
        case CREATE:
            return initialize_empty_project(out, in);
        case INSERT:
            return stage_main(out, in);
        case REVERT:
            return checkout(out, in);
        case COMMIT:
            return commit(out, in);
        case HELP:
            return help(out, in);
        case COMPARE:
            return delta_main(out, in);
        case SEE:
            return delta_main(out, in);
        case SET:
            return parse_set_cmd(out, in);

        case RST:
            {
                struct cache_object co;
                cache_object_init(&co);
                cache_object_clean(&co);
                exit(0);
            }

        case TAG:
            return create_tag(out, in);
        case HIST:
        {
            print_commits();
            break;
        }

        case USER:
        case INVALID:
        case UNKNOWN:
        {}
    }
    return true;
}

void gen_argv_array(struct strbuf *args, char ***argv, int *argc)
{
    int count = 1, j = 1;
    for (int i = 0; i < args->len;) {
        if (args->buf[i] == '"') {
            while (++i < args->len && args->buf[i] != '"')
                ;
            if (args->len != i) {
                i++;
                continue;
            }
        }
        if (args->buf[i] == ' ') {
            args->buf[i] = '\0';
            count++;
        }
        i++;
    }
    *argv = malloc(sizeof(char**) * (count + 1));
    for (int i = 0; i < args->len; i++) {
        if (i == 0) {
            (*argv)[j] = args->buf;
            j++;
        } else
        if (args->buf[i] == '\0') {
            (*argv)[j] = args->buf + i + 1;
            j++;
        }
    }
    *argc = count + 1;
}

void exec_cmd(enum cmd_type cmd, struct strbuf *args)
{
    int prefix = skip_command_prefix(args);
    struct strbuf real_args = STRBUF_INIT;
    int argc;
    char **argv;

    strbuf_remove(args, 0, prefix);
    gen_argv_array(args, &argv, &argc);
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

int main(int argc, char *argv[])
{
    if (argc < 2) {
        die("No arguments provided.\n");
    }
    struct strbuf args = STRBUF_INIT;
    struct core_commands *head;
    enum cmd_type t;

    join_args(&args, argc, argv);
    gen_core_commands(&head);
    t = find_command(head, &args);
    if (t == UNKNOWN) {
        fprintf(stderr, "peg: '"RED"%s"RESET"' unknown command.\n", argv[1]);
        //suggest_commands(argv[1]);
        exit(0);
    }
    exec_cmd(t, &args);
    return 0;
}
