#ifndef TYPES_H
#define TYPES_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

// Memory access functions
static inline u32 readl(u32 addr) {
    return *(volatile u32*)addr;
}

static inline void writel(u32 val, u32 addr) {
    *(volatile u32*)addr = val;
}

#endif