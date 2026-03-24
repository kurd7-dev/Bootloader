/*
 * MMC/SD card driver for ARM64
 */

#include "mmc.h"
#include "types.h"

// Arasan SDHCI registers (generic)
#define MMC_BASE 0x1c0f0000
#define SDHCI_DMA_ADDRESS    (MMC_BASE + 0x00)
#define SDHCI_BLOCK_SIZE     (MMC_BASE + 0x04)
#define SDHCI_BLOCK_COUNT    (MMC_BASE + 0x06)
#define SDHCI_ARGUMENT       (MMC_BASE + 0x08)
#define SDHCI_TRANSFER_MODE  (MMC_BASE + 0x0C)
#define SDHCI_COMMAND        (MMC_BASE + 0x0E)
#define SDHCI_RESPONSE       (MMC_BASE + 0x10)
#define SDHCI_PRESENT_STATE  (MMC_BASE + 0x24)
#define SDHCI_HOST_CONTROL   (MMC_BASE + 0x28)
#define SDHCI_CLOCK_CONTROL  (MMC_BASE + 0x2C)
#define SDHCI_INT_STATUS     (MMC_BASE + 0x30)
#define SDHCI_INT_ENABLE     (MMC_BASE + 0x34)
#define SDHCI_SIGNAL_ENABLE  (MMC_BASE + 0x38)

// Commands
#define CMD0  0
#define CMD2  2
#define CMD3  3
#define CMD7  7
#define CMD8  8
#define CMD17 17
#define CMD24 24

int mmc_init(void) {
    // Basic SD card initialization
    // This is a simplified version - real implementation needs full SD protocol
    
    // Enable clock
    writel(0x0001, SDHCI_CLOCK_CONTROL);
    
    // Wait for clock stable
    while (!(readl(SDHCI_CLOCK_CONTROL) & 0x2));
    
    // Send CMD0 (GO_IDLE_STATE)
    mmc_send_cmd(CMD0, 0);
    
    // Send CMD8 (SEND_IF_COND)
    mmc_send_cmd(CMD8, 0x1AA);
    
    // Simplified - assume card is ready
    return 0;
}

int mmc_send_cmd(u32 cmd, u32 arg) {
    writel(arg, SDHCI_ARGUMENT);
    writel(cmd << 8, SDHCI_COMMAND);
    
    // Wait for command complete
    while (!(readl(SDHCI_INT_STATUS) & 0x1));
    
    // Clear interrupt
    writel(0x1, SDHCI_INT_STATUS);
    
    return 0;
}

int mmc_read_block(u32 block_addr, void *dst, u32 num_blocks) {
    // Set block size (512 bytes)
    writel(512, SDHCI_BLOCK_SIZE);
    writel(num_blocks, SDHCI_BLOCK_COUNT);
    
    // Set DMA address
    writel((u32)dst, SDHCI_DMA_ADDRESS);
    
    // Send CMD17 (READ_SINGLE_BLOCK) or CMD18 for multiple
    mmc_send_cmd(CMD17, block_addr);
    
    // Wait for transfer complete
    while (!(readl(SDHCI_INT_STATUS) & 0x2));
    
    // Clear interrupt
    writel(0x2, SDHCI_INT_STATUS);
    
    return 0;
}