#include "lib/string.h"

usize bl_strlen(const char *s)
{
    usize n = 0;
    while (s && s[n]) {
        n++;
    }
    return n;
}

int bl_strcmp(const char *a, const char *b)
{
    while (*a && (*a == *b)) {
        a++;
        b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

int bl_strncmp(const char *a, const char *b, usize n)
{
    usize i;
    for (i = 0; i < n; i++) {
        unsigned char ca = (unsigned char)a[i];
        unsigned char cb = (unsigned char)b[i];
        if (ca != cb || ca == '\0' || cb == '\0') {
            return ca - cb;
        }
    }
    return 0;
}

char *bl_strcpy(char *dst, const char *src)
{
    char *ret = dst;
    while ((*dst++ = *src++) != '\0') {}
    return ret;
}

char *bl_strncpy(char *dst, const char *src, usize n)
{
    usize i;
    for (i = 0; i < n && src[i]; i++) {
        dst[i] = src[i];
    }
    for (; i < n; i++) {
        dst[i] = '\0';
    }
    return dst;
}

void *bl_memcpy(void *dst, const void *src, usize n)
{
    u8 *d = (u8 *)dst;
    const u8 *s = (const u8 *)src;
    usize i;
    for (i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dst;
}

void *bl_memset(void *dst, int c, usize n)
{
    u8 *d = (u8 *)dst;
    usize i;
    for (i = 0; i < n; i++) {
        d[i] = (u8)c;
    }
    return dst;
}

char *bl_strchr(const char *s, int c)
{
    while (*s) {
        if (*s == (char)c) {
            return (char *)s;
        }
        s++;
    }
    if (c == 0) {
        return (char *)s;
    }
    return NULL;
}

bool bl_is_space(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

u32 bl_atou32(const char *s)
{
    u32 out = 0;
    while (*s >= '0' && *s <= '9') {
        out = out * 10U + (u32)(*s - '0');
        s++;
    }
    return out;
}