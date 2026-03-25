#include "bootloader.h"
#include "bootloader_ext.h"
#include "drivers/uart_pl011.h"
#include "lib/string.h"

#define ENV_MAX_VARS  24
#define ENV_KEY_LEN   32
#define ENV_VAL_LEN   160

typedef struct {
    bool used;
    char key[ENV_KEY_LEN];
    char value[ENV_VAL_LEN];
} env_var_t;

static env_var_t g_env[ENV_MAX_VARS];

static int env_find(const char *key)
{
    usize i;
    for (i = 0; i < ARRAY_SIZE(g_env); i++) {
        if (g_env[i].used && bl_strcmp(g_env[i].key, key) == 0) {
            return (int)i;
        }
    }
    return -1;
}

void env_init(void)
{
    bl_memset(g_env, 0, sizeof(g_env));
    (void)env_set("bootdelay", "3");
    (void)env_set("bootcmd", "boot");
    (void)env_set("bootargs", DEFAULT_BOOTARGS);
    log_ok("Environment initialized");
}

const char *env_get(const char *key)
{
    int idx = env_find(key);
    if (idx < 0) {
        return NULL;
    }
    return g_env[idx].value;
}

int env_set(const char *key, const char *value)
{
    usize i;
    int idx;

    if (!key || !*key || !value) {
        return -1;
    }

    idx = env_find(key);
    if (idx < 0) {
        for (i = 0; i < ARRAY_SIZE(g_env); i++) {
            if (!g_env[i].used) {
                idx = (int)i;
                g_env[i].used = true;
                break;
            }
        }
    }

    if (idx < 0) {
        return -1;
    }

    bl_strncpy(g_env[idx].key, key, ENV_KEY_LEN - 1);
    g_env[idx].key[ENV_KEY_LEN - 1] = '\0';
    bl_strncpy(g_env[idx].value, value, ENV_VAL_LEN - 1);
    g_env[idx].value[ENV_VAL_LEN - 1] = '\0';
    return 0;
}

void env_print_all(void)
{
    usize i;
    for (i = 0; i < ARRAY_SIZE(g_env); i++) {
        if (!g_env[i].used) {
            continue;
        }
        log_info(g_env[i].key);
        uart_pl011_puts("      = ");
        uart_pl011_puts(g_env[i].value);
        uart_pl011_puts("\n");
    }
}

int env_run(const char *key)
{
    const char *cmd = env_get(key);
    if (!cmd) {
        uart_pl011_puts("env not found: ");
        uart_pl011_puts(key);
        uart_pl011_puts("\n");
        return -1;
    }
    return commands_execute_line(cmd, false);
}

void env_save_stub(void)
{
    uart_pl011_puts("saveenv: persistent storage backend not implemented yet\n");
}

int env_get_bootdelay(void)
{
    const char *s = env_get("bootdelay");
    if (!s || !*s) {
        return 3;
    }
    return (int)bl_atou32(s);
}