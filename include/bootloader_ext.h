#ifndef BOOTLOADER_EXT_H
#define BOOTLOADER_EXT_H

#include "types.h"

/* env */
void env_init(void);
const char *env_get(const char *key);
int env_set(const char *key, const char *value);
void env_print_all(void);
int env_run(const char *key);
void env_save_stub(void);
int env_get_bootdelay(void);

/* bootstate */
typedef struct {
    bool boot_in_progress;
    bool boot_success;
    u32 fail_count;
    u32 fail_limit;
    char mode[16];
} bootstate_t;

void bootstate_init(void);
void bootstate_on_boot_start(void);
void bootstate_on_boot_failure(void);
void bootstate_mark_success(void);
bool bootstate_should_enter_recovery(void);
void bootstate_print(void);
void recovery_menu_run(void);
const bootstate_t *bootstate_get(void);

/* commands/shell */
void shell_run(void);
int commands_execute_line(const char *line, bool from_history);
void commands_history_print(void);
const char *commands_history_last(void);
void commands_history_add(const char *line);

/* dtb */
bool dtb_is_valid(const void *dtb);
void *dtb_select(u64 incoming_dtb);
int dtb_patch_bootargs(void *dtb, const char *bootargs);
bool dtb_has_embedded_blob(void);

/* future expansion stubs */
int bootstate_persistent_backend_init(void);
int ramdisk_support_init(void);
int partition_gpt_init(void);
int fastboot_init(void);
int framebuffer_splash_init(void);
int usb_stack_init(void);
int filesystem_loader_init(void);

#endif