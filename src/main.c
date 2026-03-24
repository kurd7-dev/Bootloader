/*
 * Custom Baremetal Bootloader for ARM64 Android Kernel
 * 
 * This bootloader initializes hardware, loads the Android kernel from storage,
 * and boots it with proper ARM64 kernel protocol.
 */

#include "uart.h"
#include "mmc.h"
#include "types.h"

// Kernel load address (2MB aligned)
#define KERNEL_LOAD_ADDR 0x80080000
#define DTB_LOAD_ADDR    0x80000000
#define INITRD_LOAD_ADDR 0x81000000

// Storage layout (SD card offsets)
#define KERNEL_OFFSET    0x100000   // 1MB
#define DTB_OFFSET       0x200000   // 2MB  
#define INITRD_OFFSET    0x300000   // 3MB

// Boot arguments
#define CMDLINE "console=ttyAMA0,115200 androidboot.hardware=generic_arm64"

// Function prototypes
void init_hardware(void);
int load_kernel(void);
void boot_kernel(u64 kernel_addr, u64 dtb_addr);

int main(void) {
    uart_init();
    uart_puts("Custom ARM64 Bootloader starting...\n");
    
    init_hardware();
    
    if (load_kernel() != 0) {
        uart_puts("Failed to load kernel!\n");
        while (1);
    }
    
    uart_puts("Booting kernel...\n");
    boot_kernel(KERNEL_LOAD_ADDR, DTB_LOAD_ADDR);
    
    // Should not reach here
    uart_puts("Boot failed!\n");
    while (1);
}

void init_hardware(void) {
    uart_puts("Initializing hardware...\n");
    
    // Initialize MMC
    if (mmc_init() != 0) {
        uart_puts("MMC init failed!\n");
        while (1);
    }
    
    uart_puts("Hardware initialized.\n");
}

int load_kernel(void) {
    uart_puts("Loading kernel from storage...\n");
    
    // Load kernel image
    if (mmc_read_block(KERNEL_OFFSET / 512, (void*)KERNEL_LOAD_ADDR, 0x100000 / 512) != 0) {
        uart_puts("Failed to load kernel image!\n");
        return -1;
    }
    
    // Load DTB
    if (mmc_read_block(DTB_OFFSET / 512, (void*)DTB_LOAD_ADDR, 0x100000 / 512) != 0) {
        uart_puts("Failed to load DTB!\n");
        return -1;
    }
    
    // Load initrd (optional)
    if (mmc_read_block(INITRD_LOAD_ADDR / 512, (void*)INITRD_LOAD_ADDR, 0x100000 / 512) != 0) {
        uart_puts("Failed to load initrd!\n");
        // Not fatal
    }
    
    uart_puts("Kernel loaded successfully.\n");
    return 0;
}

void boot_kernel(u64 kernel_addr, u64 dtb_addr) {
    // ARM64 kernel boot protocol
    // x0 = DTB address
    // x1-x3 = 0
    // Jump to kernel with MMU off, interrupts masked
    
    asm volatile(
        "mov x0, %0\n"           // DTB address
        "mov x1, xzr\n"          // Reserved
        "mov x2, xzr\n"          // Reserved  
        "mov x3, xzr\n"          // Reserved
        "br %1\n"                // Jump to kernel
        : : "r"(dtb_addr), "r"(kernel_addr)
    );
}