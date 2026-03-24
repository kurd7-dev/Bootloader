#ifndef MMC_H
#define MMC_H

int mmc_init(void);
int mmc_send_cmd(u32 cmd, u32 arg);
int mmc_read_block(u32 block_addr, void *dst, u32 num_blocks);

#endif