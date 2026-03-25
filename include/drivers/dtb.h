#ifndef DRIVERS_DTB_H
#define DRIVERS_DTB_H

#include "types.h"

bool dtb_is_valid(const void *dtb);
void *dtb_select(u64 incoming_dtb);
int dtb_patch_bootargs(void *dtb, const char *bootargs);
bool dtb_has_embedded_blob(void);

#endif