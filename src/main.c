#include "commit.h"
#include "stage.h"
#include "delta.h"
#include "path.h"
#include "project-init.h"
#include "global.h"
#include "show-tables.h"
#include "help.h"

#include <math.h>

enum cmd_type {
    CREATE,
    INSERT,
    COMMIT,
    REVERT,
    RST,
    HIST,
    HEAD,
    STATUS,
    SHOW,
    COMPARE,
    CHECKOUT,
    SEE,
    TAG,
    SET,
    LIST,
    HELP,
    REPLAY,
    INVALID,
    USER,
    UNKNOWN
};

#define CREATE_DESC "create a peg repository"
#define INSERT_DESC "insert the files into stage area or temporary database"
#define COMMIT_DESC "commit the changes"
#define RESET_DESC  "remove the files present in the stage area"
#define REVERT_DESC "revert the local changes"
#define SHOW_DESC   "displays the tables"
#define COMPARE_DESC "shows the changes made till the last commit"
#define STATUS_DESC "shows the status of the project directory"
#define HIST_DESC "logs the commits"
#define LIST_DESC "displays the files in the last commit"
#define TAG_DESC "apply a tag to a commit"
#define HELP_DESC "shows the help for a command"
#define HEAD_DESC "change the position of the head pointer"
#define CHECKOUT_DESC "apply the version change command"
#define REPLAY_DESC "replay the changes of a repository"

struct command_type {
    enum cmd_type t;
    char *cmd;
    char *desc;
};

static struct command_type cmds[] = {
    { CREATE, "create", CREATE_DESC },
    { INSERT, "insert", INSERT_DESC },
    { COMMIT, "commit", COMMIT_DESC },
    { RST, "reset", RESET_DESC },
    { HIST, "hist", HIST_DESC },
    { HEAD, "head", HEAD_DESC },
    { STATUS, "status", STATUS_DESC },
    { SHOW, "show", SHOW_DESC },
    { REVERT, "revert", REVERT_DESC },
    { COMPARE, "compare", COMPARE_DESC },
    { CHECKOUT, "checkout", CHECKOUT_DESC },
    { LIST, "list", LIST_DESC },
    { TAG, "tag", TAG_DESC },
    { REPLAY, "replay", REPLAY_DESC },
    { HELP, "help", HELP_DESC },
    { SEE, "seeChanges" },
    { SET, "setAlias" }
};

#define COMMAND_COUNT 16

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

extern void reset_head_command(int argc, char *argv[]);
extern int list_index(int argc, char *argv[]);

void init_command(struct command *cmd)
{
    strbuf_init(&cmd->name, 0);
    cmd->type = UNKNOWN;
    cmd->argc = 0;
    cmd->argv = NULL;
}


void warn_no_user_config()
{
    fprintf(stderr, YELLOW"warning"RESET": no user name provided.\n");
}

void create_peg_environment()
{
    struct config_list *node;

    create_environment(&environment);
    read_config_file(&environment);
    parse_config_file(&environment);
    node = get_environment_value(&environment, "username");

    if (!node) {
        warn_no_user_config();
        exit(-1);
    }
    environment.owner = node->value.buf;
    node = get_environment_value(&environment, "useremail");

    if (!node) {
        die("no email provided in %s\n", environment.peg_config_filepath);
    }
    environment.owner_email = node->value.buf;
    node = get_environment_value(&environment, "password");

    if (node)
        environment.pswd = true;
}

void set_environment_value(const char *key, const char *value)
{

}

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
    if (argc == 1) {
        usage("peg help <command>\n"
        " (`peg --help` displays all the peg commands)\n");
        return true;
    }
    display_help(argv[1]);
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

        case SHOW:
            return show_tables(count, in);

        case HEAD:
            reset_head_command(count, in);
            return 0;

        case CHECKOUT:
            status(".");
            revert_files_hard();
            return 0;

        case REPLAY:
            system("bash replay");
            exit(0);

        case USER:
        case INVALID:
        case UNKNOWN:
        {}
    }
    return 0;
}

void exec_cmd(enum cmd_type cmd, int argc, char **argv)
{
    exec_commands_args(cmd, argc, argv);
}

void print_help()
{
    printf("Available `peg` commands: \n");
    for (int i = 0; i < COMMAND_COUNT - 2; i++) {
        printf("  "YELLOW"%10s"RESET"\t%s\n", cmds[i].cmd, cmds[i].desc);
    }
    printf("( use <PEG HELP ALL> to get a list of all \ncommands with their syntax and functionality)\n ");
}

//Suggetion System
int levenshtein(const char *string1, const char *string2,
        int w, int s, int a, int d)
{
    int len1 = strlen(string1), len2 = strlen(string2);
    int *row0 = malloc(sizeof(int) * (len2 + 1));
    int *row1 = malloc(sizeof(int) * (len2 + 1));
    int *row2 = malloc(sizeof(int) * (len2 + 1));
    int i, j;

    for (j = 0; j <= len2; j++)
        row1[j] = j * a;
    for (i = 0; i < len1; i++) {
        int *dummy;

        row2[0] = (i + 1) * d;
        for (j = 0; j < len2; j++) {
            /* substitution */
            row2[j + 1] = row1[j] + s * (string1[i] != string2[j]);
            /* swap */
            if (i > 0 && j > 0 && string1[i - 1] == string2[j] &&
                    string1[i] == string2[j - 1] &&
                    row2[j + 1] > row0[j - 1] + w)
                row2[j + 1] = row0[j - 1] + w;
            /* deletion */
            if (row2[j + 1] > row1[j + 1] + d)
                row2[j + 1] = row1[j + 1] + d;
            /* insertion */
            if (row2[j + 1] > row2[j] + a)
                row2[j + 1] = row2[j] + a;
        }

        dummy = row0;
        row0 = row1;
        row1 = row2;
        row2 = dummy;
    }

    i = row1[len2];
    free(row0);
    free(row1);
    free(row2);

    return i;
}
//

enum cmd_type suggest_commands(char *cmdbuff,struct core_commands *head)
{   struct core_commands *node=head;
    int w,s,a,d,min=100000;
    struct core_commands *indexsp;
    w=s=a=d=strlen(cmdbuff);
    while(node!=NULL)
    {
    node->cmd.match = levenshtein(cmdbuff, node->cmd.name.buf,w,s,a,d);
    if((node->cmd.match)<min)
    {
            min=node->cmd.match;
            indexsp=node;

        }
        node=node->next;
    }
    fprintf(stderr,"%s is not a valid peg command\nDid you mean?\n\t",cmdbuff);
    fprintf(stderr, "%s\n(y/n)\n",indexsp->cmd.name.buf);
    char ch;
    ch=getchar();
    if(tolower(ch)=='y')
        return indexsp->cmd.type;
    else
        return UNKNOWN;
}


int main(int argc, char *argv[])
{
    struct core_commands *head;
    enum cmd_type t;

    if (argc == 1 || !strcmp(argv[1], "--help")) {
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

    create_peg_environment();
    gen_core_commands(&head);
    t = find_command(head, argv[1]);
    if (t == UNKNOWN) {

        t=suggest_commands(argv[1],head);

        if(t == UNKNOWN)
        exit(0);
    }
    exec_cmd(t, --argc, (argv + 1));
    return 0;
}
