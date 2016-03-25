#include "util.h"
#include "project-init.h"
#include "timestamp.h"
#include "project-config.h"
#include "visitor.h"

#ifdef _WIN32
#include <windows.h>
#endif

struct project_cache {
    struct project_details pd;
    struct timestamp ts;
};

int name_set, desc_set;

void project_cache_init(struct project_cache *pc)
{
    project_details_init(&pc->pd);
    time_stamp_init(&pc->ts);
}

bool cache_valid(struct project_cache *pc)
{
    if (pc->pd.project_name.len == 0) return false;
    return true;
}

int make_peg_directories()
{
    struct visitor vt;
    visitor_init(&vt);
    visitor_visit(&vt, ".");
    if (visitor_make_folder(&vt, PEG_DIR) < 0) return -1;
#ifdef _WIN32
    SetFileAttributes(PEG_DIR, 0x2);
#endif

    visitor_close(&vt);
    visitor_init(&vt);
    visitor_visit(&vt, PEG_DIR);
    if (visitor_make_folder(&vt, CACHE_DIR) < 0) die("error occurred\n");
    if (visitor_make_folder(&vt, DB_DIR) < 0) die("error occurred\n");
    if (visitor_make_folder(&vt, COMMIT_DIR) < 0) die("error occurred\n");
    visitor_close(&vt);
    return 0;
}

int create_description_file(struct project_cache *pc)
{
    FILE *desc_file = fopen(DESCRIPTION_FILE, "w");
    if (!desc_file) {
        fprintf(stderr, "failed to create the description file. %s\n",
                strerror(errno));
        return -1;
    }
    struct strbuf buf = STRBUF_INIT;
    strbuf_addstr(&buf, "ProjectName: ");
    strbuf_addbuf(&buf, &pc->pd.project_name);
    strbuf_addstr(&buf, "\nProjectDescription: ");
    strbuf_addbuf(&buf, &pc->pd.project_desc);
    strbuf_addstr(&buf, "\nAuthor:");
    strbuf_addbuf(&buf, &pc->pd.authors);
    strbuf_addstr(&buf, "\nTimeStarted: ");
    strbuf_addstr(&buf, asctime(pc->ts._tm));
    fprintf(desc_file, "%s\n", buf.buf);
    strbuf_release(&buf);
    fclose(desc_file);
    return 0;
}

void create_cache_files(struct project_cache *pc)
{
    size_t size = 0;
    FILE *cache = fopen(CACHE_PACK_FILE, "w");
    FILE *idx = fopen(CACHE_INDEX_FILE, "w");

    if (!cache || !idx)
        die("unable to create cache file, %s\n", strerror(errno));
    fwrite(&size, sizeof(size_t), 1, idx);
    fclose(cache);
    fclose(idx);
}

void create_database_files(struct project_cache *pc)
{
    size_t size = 0;
    FILE *pack = fopen(COMMIT_INDEX_FILE, "w");
    if (!pack)
        die("unable to open %s, %s\n", COMMIT_INDEX_FILE, strerror(errno));
    fwrite(&size, sizeof(size_t), 1, pack);
    fclose(pack);
    pack = fopen(FILE_INDEX_FILE, "w");
    if (!pack) die("unable to open %s\n", FILE_INDEX_FILE);
    fclose(pack);
    pack = fopen(PACK_FILE, "w");
    if (!pack) die("unable to open %s\n", PACK_FILE);
    fclose(pack);
}

const char *init_usage = PEG_NAME
    " init [options] [VALUES]\n"
    "    Initialise a project.\n"
    "\n"
    "    options: \n"
    "        -n, --name [NAME]             Name of the project\n"
    "        -d, --desc [DESCRIPTION]      Description of the project. Can be "
    "empty\n"
    "        -a, --author [AUTHOR_NAME]    Name of the author. Default will "
    "empty"
    "                                      name or one specified in your global"
    "                                      config file.\n"
    "        -h, --help                    Display the above information\n";

int parse_single_argument(struct project_cache *pc, int argc, char *argv[])
{
    if ((!strcmp(argv[argc], "-n") || !strcmp(argv[argc], "--name")) &&
        !name_set) {
        set_project_name(&pc->pd, argv[argc + 1]);
        name_set = 1;
        return 0;
    }

    if (!strcmp(argv[argc], "-a") || !strcmp(argv[argc], "--author")) {
        set_project_author(&pc->pd, argv[argc + 1]);
        return 0;
    }

    if (!strcmp(argv[argc], "-d") || !strcmp(argv[argc], "--desc")) {
        struct strbuf temp = STRBUF_INIT;
        strbuf_addstr(&temp, argv[argc + 1]);
        set_project_description(&pc->pd, &temp, 0);
        strbuf_release(&temp);
        return 0;
    }
    if (name_set) return 0;
    fprintf(stderr, "bad arguments\n See " PEG_NAME " init --help for usage");
    exit(0);
}

static int parse_arguments(struct project_cache *pc, int argc, char *argv[])
{
    if (argc < 2) die("needed atleast one argument. Use -h for usage.\n");
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            fprintf(stderr, "%s", init_usage);
            exit(0);
        }
    }

    if (argv[1][0] != '-' && !name_set) {
        set_project_name(&pc->pd, argv[1]);
        name_set = 1;
    }

    for (int i = 1; i < argc; i++) parse_single_argument(pc, i, argv);

    if (!cache_valid(pc)) {
        fatal("error specify a project name.\n");
        return -1;
    }
    return 0;
}

int initialize_empty_project(int argc, char *argv[])
{
    struct project_cache pc;

    delayms(100000);
    project_cache_init(&pc);
    if (parse_arguments(&pc, argc, argv) < 0) return -1;
    if (make_peg_directories() < 0) return -1;
    if (!create_description_file(&pc))
        fprintf(stderr, BOLD_GREEN"Initialised an empty project\n"RESET);
    create_cache_files(&pc);
    create_database_files(&pc);
    delayms(200000);
    return 0;
}
