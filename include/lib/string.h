#ifndef BL_STRING_H
#define BL_STRING_H

#include "types.h"

usize bl_strlen(const char *s);
int bl_strcmp(const char *a, const char *b);
int bl_strncmp(const char *a, const char *b, usize n);
char *bl_strcpy(char *dst, const char *src);
char *bl_strncpy(char *dst, const char *src, usize n);
void *bl_memcpy(void *dst, const void *src, usize n);
void *bl_memset(void *dst, int c, usize n);
char *bl_strchr(const char *s, int c);
bool bl_is_space(char c);
u32 bl_atou32(const char *s);

#endif