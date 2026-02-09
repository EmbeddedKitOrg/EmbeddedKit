#ifndef EK_STR_H
#define EK_STR_H

#include "../../../ek_conf.h"

#if EK_STR_ENABLE == 1

#    include "ek_def.h"

typedef struct ek_str_t ek_str_t;

struct ek_str_t
{
    char *buf;
    uint32_t cap;
    uint32_t len;
};

ek_str_t *ek_str_create(const char *str);
void ek_str_free(ek_str_t *s);
void ek_str_clear(ek_str_t *s);

bool ek_str_append_len(ek_str_t *s, const char *str, uint32_t len);
bool ek_str_append(ek_str_t *s, const char *str);
bool ek_str_append_fmt(ek_str_t *s, const char *fmt, ...);

bool ek_str_cat(ek_str_t *dst, ek_str_t *src);
ek_str_t *ek_str_slice(const ek_str_t *s, int32_t start, int32_t end);
const char *ek_str_get_cstring(ek_str_t *s);

uint32_t ek_str_get_len(ek_str_t *s);
uint32_t ek_str_get_cap(ek_str_t *s);

int ek_str_cmp(ek_str_t *s1, ek_str_t *s2);
int ek_str_ncmp(ek_str_t *s1, ek_str_t *s2, size_t n);

#endif // EK_STR_ENABLE

#endif // EK_STR_H
