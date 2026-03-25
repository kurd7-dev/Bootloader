#include "bootloader.h"
#include "bootloader_ext.h"
#include "drivers/uart_pl011.h"
#include "lib/string.h"

#define CMD_MAX_LINE   160
#define CMD_MAX_TOKENS 8

#define HISTORY_MAX       16
#define HISTORY_LINE_MAX  160

static char g_history[HISTORY_MAX][HISTORY_LINE_MAX];
static u32 g_history_count;

static int split_tokens(char *line, char *argv[], int max_tokens)
{
    int argc = 0;
    while (*line && argc < max_tokens) {
        while (*line && bl_is_space(*line)) {
            *line++ = '\0';
        }
        if (!*line) {
            break;
        }
        argv[argc++] = line;
        while (*line && !bl_is_space(*line)) {
            line++;
        }
    }
    return argc;
}

void commands_history_add(const char *line)
{
    u32 idx;
    if (!line || !*line) {
        return;
    }
    idx = g_history_count % HISTORY_MAX;
    bl_strncpy(g_history[idx], line, HISTORY_LINE_MAX - 1);
    g_history[idx][HISTORY_LINE_MAX - 1] = '\0';
    g_history_count++;
}

const char *commands_history_last(void)
{
    if (g_history_count == 0) {
        return NULL;
    }
    return g_history[(g_history_count - 1) % HISTORY_MAX];
}

void commands_history_print(void)
{
    u32 i;
    u32 start = (g_history_count > HISTORY_MAX) ? (g_history_count - HISTORY_MAX) : 0;
    for (i = start; i < g_history_count; i++) {
        bl_putdec(i + 1);
        uart_pl011_puts(": ");
        uart_pl011_puts(g_history[i % HISTORY_MAX]);
        uart_pl011_puts("\n");
    }
}

static void print_help(void)
{
    uart_pl011_puts("Commands:\n");
    uart_pl011_puts("  help                 - show this help\n");
    uart_pl011_puts("  history              - show command history\n");
    uart_pl011_puts("  !!                   - repeat last command\n");
    uart_pl011_puts("  boot                 - try normal boot\n");
    uart_pl011_puts("  reset                - reboot via PSCI\n");
    uart_pl011_puts("  printenv             - print environment\n");
    uart_pl011_puts("  setenv K V           - set environment variable\n");
    uart_pl011_puts("  run K                - run command stored in env key\n");
    uart_pl011_puts("  saveenv              - persistence stub\n");
    uart_pl011_puts("  bootstate            - print boot state\n");
    uart_pl011_puts("  bootsuccess          - mark boot success\n");
    uart_pl011_puts("  drvinfo              - print driver info\n");
    uart_pl011_puts("  recovery             - open recovery menu\n");
}

int commands_execute_line(const char *line, bool from_history)
{
    char buf[CMD_MAX_LINE];
    char *argv[CMD_MAX_TOKENS];
    int argc;

    if (!line) {
        return -1;
    }

    bl_strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    while (*buf && bl_is_space(*buf)) {
        usize i;
        for (i = 0; buf[i]; i++) {
            buf[i] = buf[i + 1];
        }
    }
    if (!*buf) {
        return 0;
    }

    if (bl_strcmp(buf, "!!") == 0) {
        const char *last = commands_history_last();
        if (!last) {
            uart_pl011_puts("No history\n");
            return -1;
        }
        uart_pl011_puts(last);
        uart_pl011_puts("\n");
        return commands_execute_line(last, true);
    }

    if (!from_history) {
        commands_history_add(buf);
    }

    argc = split_tokens(buf, argv, CMD_MAX_TOKENS);
    if (argc == 0) {
        return 0;
    }

    if (bl_strcmp(argv[0], "help") == 0) {
        print_help();
        return 0;
    }
    if (bl_strcmp(argv[0], "history") == 0) {
        commands_history_print();
        return 0;
    }
    if (bl_strcmp(argv[0], "boot") == 0) {
        return boot_try_normal();
    }
    if (bl_strcmp(argv[0], "reset") == 0) {
        boot_reset();
        return 0;
    }
    if (bl_strcmp(argv[0], "printenv") == 0) {
        env_print_all();
        return 0;
    }
    if (bl_strcmp(argv[0], "setenv") == 0) {
        if (argc < 3) {
            uart_pl011_puts("usage: setenv <key> <value>\n");
            return -1;
        }
        if (env_set(argv[1], argv[2]) != 0) {
            uart_pl011_puts("setenv failed\n");
            return -1;
        }
        return 0;
    }
    if (bl_strcmp(argv[0], "run") == 0) {
        if (argc < 2) {
            uart_pl011_puts("usage: run <key>\n");
            return -1;
        }
        return env_run(argv[1]);
    }
    if (bl_strcmp(argv[0], "saveenv") == 0) {
        env_save_stub();
        return 0;
    }
    if (bl_strcmp(argv[0], "bootstate") == 0) {
        bootstate_print();
        return 0;
    }
    if (bl_strcmp(argv[0], "bootsuccess") == 0) {
        bootstate_mark_success();
        uart_pl011_puts("boot success marked\n");
        return 0;
    }
    if (bl_strcmp(argv[0], "drvinfo") == 0) {
        drvinfo_print();
        return 0;
    }
    if (bl_strcmp(argv[0], "recovery") == 0) {
        recovery_menu_run();
        return 0;
    }

    uart_pl011_puts("Unknown command: ");
    uart_pl011_puts(argv[0]);
    uart_pl011_puts("\n");
    return -1;
}