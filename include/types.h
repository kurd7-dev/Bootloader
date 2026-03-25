#ifndef TYPES_H
#define TYPES_H

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef signed char        s8;
typedef signed short       s16;
typedef signed int         s32;
typedef signed long long   s64;

typedef unsigned long      usize;

typedef int                bool;

#define true  1
#define false 0

#ifndef NULL
#define NULL ((void*)0)
#endif

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static inline u32 readl(u64 addr)
{
    return *(volatile u32 *)(addr);
}

static inline void writel(u32 val, u64 addr)
{
    *(volatile u32 *)(addr) = val;
}

static inline u64 readq(u64 addr)
{
    return *(volatile u64 *)(addr);
}

static inline void writeq(u64 val, u64 addr)
{
    *(volatile u64 *)(addr) = val;
}

#endif