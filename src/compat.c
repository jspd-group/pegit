#include "util.h"

const char *_colors[] = {
    "\x1b[30m",   "\x1b[31m",   "\x1b[32m",   "\x1b[33m",   "\x1b[34m",
    "\x1b[35m",   "\x1b[36m",   "\x1b[37m",   "\x1b[0;0m",  "\x1b[31;32m",
    "\x1b[90m",   "\x1b[1m",    "\x1b[1;31m", "\x1b[1;32m", "\x1b[1;33m",
    "\x1b[1;34m", "\x1b[1;35m", "\x1b[1;36m", "\x1b[1;37m", "\x1b[41m",
    "\x1b[42m",   "\x1b[43m",   "\x1b[44m",   "\x1b[45m",   "\x1b[46m"};

const char *default_output[] = {"", "", "", "", "", "", "", "", "", "", "", "",
                                "", "", "", "", "", "", "", "", "", "", ""};

char **colors;

void set_non_colored_output() { colors = (char *)default_output; }

void clean_colors(char **colors, int i)
{
    while (i--) {
        free(colors[i]);
    }

    // this buddy was left, so remove it too!
    free(colors[0]);
    free(colors);
}

void set_colored_output()
{
    colors = malloc(sizeof(char *) * COLOR_COUNT);

    if (!colors) {
        // fallback to non colored output;
        set_non_colored_output();
        return;
    }

    for (int i = 0; i < COLOR_COUNT; i++) {
        colors[i] = malloc(12);

        if (!colors[i]) {
            clean_colors(colors, i - 1);
            set_non_colored_output();
            return;
        }
        strcpy(colors[i], _colors[i]);
    }
}

void set_colors()
{
    // so we'll check whether the output is redirected or not
    if (isatty(STDOUT_FILNO)) {
        // we're definitely using console as output
        set_colored_output();
    } else {
        set_non_colored_output();
    }
}
