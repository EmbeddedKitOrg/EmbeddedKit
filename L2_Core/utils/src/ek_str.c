/**
 * @file ek_str.c
 * @brief 动态字符串管理实现
 * @author N1netyNine99
 */

#include "../inc/ek_str.h"

#if EK_STR_ENABLE == 1

#    include "../inc/ek_io.h"
#    include "../inc/ek_mem.h"
#    include "../inc/ek_assert.h"

#    define INDEX_CLAMP(idx, len)             \
        do                                    \
        {                                     \
            if ((idx) < 0) (idx) += (len);    \
            if ((idx) < 0) (idx) = 0;         \
            if ((idx) > (len)) (idx) = (len); \
        } while (0)

static bool _ek_str_ensure_cap(ek_str_t *s, uint32_t len)
{
    ek_assert_tiny(s != NULL);

    if (s->cap >= len) return true;

    uint32_t new_cap = s->cap;
    do
    {
        if (new_cap == 0) new_cap = 2;
        new_cap += new_cap / 2;
    } while (new_cap < len);

    char *buf = NULL;

    if (s->buf == NULL) buf = ek_malloc(len);
    else buf = ek_realloc(s->buf, new_cap);

    if (buf == NULL) return false;

    s->buf = buf;
    s->cap = new_cap;

    return true;
}

ek_str_t *ek_str_create(const char *str)
{
    ek_str_t *s = ek_malloc(sizeof(ek_str_t));
    if (s == NULL) return NULL;

    s->buf = NULL;
    s->cap = 0;
    s->len = 0;

    // 传入 NULL 则创建一个空的字符串
    if (str == NULL) return s;

    if (ek_str_append(s, str) == false)
    {
        ek_free(s);
        return NULL;
    }

    return s;
}

void ek_str_free(ek_str_t *s)
{
    ek_assert_tiny(s != NULL);

    s->cap = 0;
    s->len = 0;
    ek_free(s->buf);
    ek_free(s);
}

void ek_str_clear(ek_str_t *s)
{
    ek_assert_tiny(s != NULL);

    s->len = 0;
}

bool ek_str_append_len(ek_str_t *s, const char *str, uint32_t len)
{
    ek_assert_tiny(s != NULL);

    // 如果传入追加内容为空，并且字符长度也为空
    // 什么也不做
    if (str == NULL && len == 0) return true;

    ek_assert_tiny(str != NULL);
    ek_assert_tiny(len != 0);

    if (_ek_str_ensure_cap(s, s->len + len + 1) == false) return false;
    memcpy(s->buf + s->len, str, len);

    s->len += len;
    s->buf[s->len] = '\0';

    return true;
}

bool ek_str_append_fmt(ek_str_t *s, const char *fmt, ...)
{
    ek_assert_tiny(s != NULL);

    // 获取格式化字符串的长度
    va_list args;
    va_start(args, fmt);
    int len = lwvsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (len < 0) return false;

    if (_ek_str_ensure_cap(s, s->len + len) == false) return false;

    va_start(args, fmt);
    // +1 给 \0
    lwvsnprintf(s->buf + s->len, len + 1, fmt, args);
    va_end(args);

    s->len += len;
    s->buf[s->len] = '\0';

    return true;
}

bool ek_str_append(ek_str_t *s, const char *str)
{
    return ek_str_append_len(s, str, strlen(str));
}

bool ek_str_cat(ek_str_t *dst, ek_str_t *src)
{
    return ek_str_append_len(dst, src->buf, src->len);
}

ek_str_t *ek_str_slice(const ek_str_t *s, int32_t start, int32_t end)
{
    ek_assert_tiny(s != NULL);

    int32_t len = (int32_t)s->len;

    INDEX_CLAMP(start, len);
    INDEX_CLAMP(end, len);

    if (start == end) return ek_str_create("");

    uint32_t new_len = (uint32_t)(end - start);

    ek_str_t *new_s = ek_str_create(NULL);
    if (new_s == NULL) return NULL;

    new_s->buf = ek_malloc(new_len + 1);
    if (new_s->buf == NULL)
    {
        ek_free(new_s);
        return NULL;
    }

    memcpy(new_s->buf, s->buf + start, new_len);
    new_s->len = new_len;
    new_s->cap = new_len;
    new_s->buf[new_s->len] = '\0';

    return new_s;
}

const char *ek_str_get_cstring(ek_str_t *s)
{
    ek_assert_tiny(s != NULL);
    if (s->buf == NULL) return "";
    return s->buf;
}

uint32_t ek_str_get_len(ek_str_t *s)
{
    ek_assert_tiny(s != NULL);

    return s->len;
}

uint32_t ek_str_get_cap(ek_str_t *s)
{
    ek_assert_tiny(s != NULL);

    return s->cap;
}

int ek_str_cmp(ek_str_t *s1, ek_str_t *s2)
{
    ek_assert_tiny(s1 != NULL);
    ek_assert_tiny(s2 != NULL);

    return strcmp(ek_str_get_cstring(s1), ek_str_get_cstring(s2));
}

int ek_str_ncmp(ek_str_t *s1, ek_str_t *s2, size_t n)
{
    ek_assert_tiny(s1 != NULL);
    ek_assert_tiny(s2 != NULL);

    uint32_t len1 = ek_str_get_len(s1);
    uint32_t len2 = ek_str_get_len(s2);

    uint32_t shorter_len = (len1 <= len2) ? len1 : len2;

    if (n > shorter_len) n = shorter_len;

    return strncmp(ek_str_get_cstring(s1), ek_str_get_cstring(s2), n);
}

#endif // EK_STR_ENABLE
