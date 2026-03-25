#include "bootloader.h"
#include "drivers/dtb.h"
#include "lib/string.h"

extern u8 __dtb_blob_start[];
extern u8 __dtb_blob_end[];

#define FDT_MAGIC 0xd00dfeedU

#define FDT_BEGIN_NODE 0x1U
#define FDT_END_NODE   0x2U
#define FDT_PROP       0x3U
#define FDT_NOP        0x4U
#define FDT_END        0x9U

typedef struct {
    u32 magic;
    u32 totalsize;
    u32 off_dt_struct;
    u32 off_dt_strings;
    u32 off_mem_rsvmap;
    u32 version;
    u32 last_comp_version;
    u32 boot_cpuid_phys;
    u32 size_dt_strings;
    u32 size_dt_struct;
} fdt_header_t;

typedef struct {
    u32 len;
    u32 nameoff;
} fdt_prop_t;

static u32 be32_to_cpu(u32 v)
{
    return ((v & 0x000000FFU) << 24) |
           ((v & 0x0000FF00U) << 8) |
           ((v & 0x00FF0000U) >> 8) |
           ((v & 0xFF000000U) >> 24);
}

static u32 align4(u32 v)
{
    return (v + 3U) & ~3U;
}

bool dtb_is_valid(const void *dtb)
{
    const fdt_header_t *h = (const fdt_header_t *)dtb;
    u32 totalsize;
    if (!dtb) {
        return false;
    }
    if (be32_to_cpu(h->magic) != FDT_MAGIC) {
        return false;
    }
    totalsize = be32_to_cpu(h->totalsize);
    if (totalsize < sizeof(fdt_header_t) || totalsize > (16U * 1024U * 1024U)) {
        return false;
    }
    return true;
}

bool dtb_has_embedded_blob(void)
{
    usize sz = (usize)(__dtb_blob_end) - (usize)(__dtb_blob_start);
    if (sz == 0) {
        return false;
    }
    return dtb_is_valid(__dtb_blob_start);
}

void *dtb_select(u64 incoming_dtb)
{
    void *p;

    p = (void *)(usize)incoming_dtb;
    if (dtb_is_valid(p)) {
        return p;
    }

    if (dtb_has_embedded_blob()) {
        return __dtb_blob_start;
    }

    p = (void *)(usize)DTB_FALLBACK_ADDR;
    if (dtb_is_valid(p)) {
        return p;
    }

    return NULL;
}

int dtb_patch_bootargs(void *dtb, const char *bootargs)
{
    fdt_header_t *h = (fdt_header_t *)dtb;
    u8 *base = (u8 *)dtb;
    u8 *struct_base;
    u8 *strings_base;
    u32 struct_size;
    u32 off = 0;
    int depth = 0;
    bool in_chosen = false;
    u32 needed;

    if (!dtb_is_valid(dtb) || !bootargs) {
        return -1;
    }

    struct_base = base + be32_to_cpu(h->off_dt_struct);
    strings_base = base + be32_to_cpu(h->off_dt_strings);
    struct_size = be32_to_cpu(h->size_dt_struct);
    needed = (u32)bl_strlen(bootargs) + 1U;

    while (off + 4U <= struct_size) {
        u32 token = be32_to_cpu(*(u32 *)(struct_base + off));
        off += 4U;

        if (token == FDT_BEGIN_NODE) {
            char *name = (char *)(struct_base + off);
            u32 nlen = (u32)bl_strlen(name) + 1U;
            depth++;
            in_chosen = (depth == 2 && bl_strcmp(name, "chosen") == 0);
            off += align4(nlen);
            continue;
        }
        if (token == FDT_END_NODE) {
            if (depth == 2 && in_chosen) {
                in_chosen = false;
            }
            if (depth > 0) {
                depth--;
            }
            continue;
        }
        if (token == FDT_PROP) {
            fdt_prop_t *prop;
            u32 len;
            u32 nameoff;
            const char *pname;
            u8 *pdata;

            if (off + sizeof(fdt_prop_t) > struct_size) {
                return -1;
            }
            prop = (fdt_prop_t *)(struct_base + off);
            len = be32_to_cpu(prop->len);
            nameoff = be32_to_cpu(prop->nameoff);
            off += sizeof(fdt_prop_t);
            pdata = struct_base + off;
            pname = (const char *)(strings_base + nameoff);

            if (in_chosen && bl_strcmp(pname, "bootargs") == 0) {
                if (len < needed) {
                    return -1;
                }
                bl_memset(pdata, 0, len);
                bl_memcpy(pdata, bootargs, needed);
                return 0;
            }
            off += align4(len);
            continue;
        }
        if (token == FDT_NOP) {
            continue;
        }
        if (token == FDT_END) {
            break;
        }
        return -1;
    }

    return -1;
}