#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Set at build time by the Makefile (-DVERSION / -DDATADIR); these are fallbacks. */
#ifndef VERSION
#define VERSION "1.0.0"
#endif
#ifndef DATADIR
#define DATADIR "/usr/share/clihelp/data"
#endif

/* ---------- colors (disabled automatically when piped or NO_COLOR is set) ---------- */
static const char *RESET = "", *BOLD = "", *DIM = "";
static const char *CYAN = "", *GREEN = "", *YELLOW = "", *MAGENTA = "", *RED = "";

static void init_colors(void)
{
    if (!isatty(STDOUT_FILENO) || getenv("NO_COLOR"))
        return;
    RESET   = "\033[0m";
    BOLD    = "\033[1m";
    DIM     = "\033[2m";
    RED     = "\033[31m";
    GREEN   = "\033[32m";
    YELLOW  = "\033[33m";
    MAGENTA = "\033[35m";
    CYAN    = "\033[36m";
}

/* ---------- categories ---------- */
typedef struct {
    const char *name;
    const char *file;
} Category;

static const Category cats[] = {
    {"One-liners",              "oneliners.txt"},
    {"System information",      "systeminformation.txt"},
    {"System control",          "systemcontrol.txt"},
    {"System Recovery",         "systemrecovery.txt"},
    {"Users & Groups",          "usersgroups.txt"},
    {"Files & Folders",         "filesfolders.txt"},
    {"Input",                   "input.txt"},
    {"Printing",                "printing.txt"},
    {"JSON",                    "json.txt"},
    {"Network",                 "network.txt"},
    {"Search & Find",           "searchfind.txt"},
    {"Git",                     "git.txt"},
    {"SSH",                     "ssh.txt"},
    {"Video & Audio",           "videoaudio.txt"},
    {"Package manager",         "packagemanager.txt"},
    {"Text Processing",         "textprocessing.txt"},
    {"Compression & Archiving", "compressionarchiving.txt"},
    {"Backup & Imaging",        "backupimaging.txt"},
    {"Tmux",                    "tmux.txt"},
};
#define NCATS ((int)(sizeof(cats) / sizeof(cats[0])))

#define COL_WIDTH 26   /* width of the name column */

/* ---------- drawing helpers ---------- */
static void print_header(void)
{
    printf("\n%s%s", BOLD, CYAN);
    printf("  ╭──────────────────────────────────────────────────────────╮\n");
    printf("  │%s                     CLIHELP                              %s%s│\n", YELLOW, CYAN, BOLD);
    printf("  │%s%s          Linux cheatsheet, right in your terminal        %s%s%s│\n", RESET, DIM, RESET, BOLD, CYAN);
    printf("  ╰──────────────────────────────────────────────────────────╯%s\n\n", RESET);
}

static void print_menu(void)
{
    int rows = (NCATS + 1) / 2;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < 2; c++) {
            int i = r + c * rows;
            if (i >= NCATS)
                break;
            /* pad using the plain name so color codes don't break alignment */
            printf("  %s%s%2d%s %s›%s %-*s",
                   BOLD, GREEN, i + 1, RESET,
                   DIM, RESET,
                   COL_WIDTH, cats[i].name);
        }
        printf("\n");
    }
    printf("\n");
}

/* ---------- highlight one shell command ----------
 * program name -> bold green      sudo         -> red
 * -flags       -> cyan            | && || ;    -> magenta
 * everything else stays default
 */
static void print_command(const char *cmd)
{
    int expect_prog = 1;
    const char *p = cmd;

    printf("    %s$%s ", DIM, RESET);
    while (*p) {
        if (*p == ' ' || *p == '\t') {
            putchar(*p++);
            continue;
        }
        const char *start = p;
        while (*p && *p != ' ' && *p != '\t')
            p++;
        int len = (int)(p - start);

        const char *weight = "";
        const char *color  = "";

        if ((len == 1 && (*start == '|' || *start == ';')) ||
            (len == 2 && (!strncmp(start, "&&", 2) || !strncmp(start, "||", 2)))) {
            color = MAGENTA;
            expect_prog = 1;
        } else if (len == 4 && !strncmp(start, "sudo", 4)) {
            color = RED;                       /* next word is still the program */
        } else if (expect_prog) {
            weight = BOLD;
            color = GREEN;
            expect_prog = 0;
        } else if (*start == '-') {
            color = CYAN;
        }
        printf("%s%s%.*s%s", weight, color, len, start, RESET);
    }
    putchar('\n');
}

/* ---------- show a data file ----------
 * '# TITLE'      -> section heading
 * '$ command'    -> highlighted command
 * anything else  -> description text
 */
/* Where are the cheatsheet files?
 *   1. $CLIHELP_DATA          (override, handy for development)
 *   2. DATADIR                (installed location, e.g. /usr/share/clihelp/data)
 *   3. ./data                 (running straight from the source folder)
 */
static const char *find_data_dir(void)
{
    const char *env = getenv("CLIHELP_DATA");
    if (env && *env)
        return env;
    if (access(DATADIR, R_OK | X_OK) == 0)
        return DATADIR;
    if (access("data", R_OK | X_OK) == 0)
        return "data";
    return NULL;
}

static int show_file(const Category *cat, const char *dir)
{
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", dir, cat->file);

    FILE *fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "%sCould not open %s%s\n", RED, path, RESET);
        return 1;
    }

    printf("\n%s%s  ▌ %s%s\n", BOLD, MAGENTA, cat->name, RESET);
    printf("  %s────────────────────────────────────────%s\n\n", DIM, RESET);

    char line[1024];
    int prev_blank = 1;               /* collapse runs of blank lines */

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = '\0';

        const char *t = line;
        while (*t == ' ' || *t == '\t')
            t++;

        if (*t == '\0') {                                  /* blank line */
            if (!prev_blank)
                putchar('\n');
            prev_blank = 1;
            continue;
        }

        if (*t == '#') {                                   /* section heading */
            t++;
            while (*t == ' ')
                t++;
            if (!prev_blank)
                putchar('\n');
            printf("  %s%s▌ %s%s\n", BOLD, YELLOW, t, RESET);
            printf("  %s──────────────────────────────%s\n", DIM, RESET);
        } else if (t[0] == '$' && (t[1] == ' ' || t[1] == '\0')) {   /* command */
            t++;
            while (*t == ' ')
                t++;
            print_command(t);
        } else {                                           /* description */
            printf("  %s\n", t);
        }
        prev_blank = 0;
    }
    printf("\n");

    fclose(fp);
    return 0;
}

int main(int argc, char **argv)
{
    char input[16];

    if (argc > 1 && (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version"))) {
        printf("clihelp %s\n", VERSION);
        return 0;
    }

    const char *dir = find_data_dir();
    if (!dir) {
        fprintf(stderr, "clihelp: cheatsheet data not found (looked in %s and ./data).\n"
                        "Set CLIHELP_DATA to the folder containing the .txt files.\n", DATADIR);
        return 1;
    }

    init_colors();
    print_header();
    print_menu();

    printf("  %sSelect a category%s %s›%s ", BOLD, RESET, CYAN, RESET);
    fflush(stdout);

    if (!fgets(input, sizeof(input), stdin))
        return 1;

    int choice = atoi(input);

    if (choice < 1 || choice > NCATS) {
        fprintf(stderr, "\n  %sInvalid choice.%s Pick a number from 1 to %d.\n", RED, RESET, NCATS);
        return 1;
    }

    return show_file(&cats[choice - 1], dir);
}
