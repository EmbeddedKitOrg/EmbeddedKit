/**
 * @file ek_str.h
 * @brief 动态字符串管理
 * @author N1netyNine99
 *
 * 提供动态增长的字符串数据结构，支持：
 * - 自动扩容
 * - 字符串追加、拼接、切片
 * - 格式化输出
 * - 字符串比较
 *
 * @note 使用前需调用 ek_str_create() 创建，使用完毕后需调用 ek_str_free() 释放
 */

#ifndef EK_STR_H
#define EK_STR_H

#include "../../../ek_conf.h"

#if EK_STR_ENABLE == 1

#    include "ek_def.h"

#    ifdef __cplusplus
extern "C"
{
#    endif

/**
 * @brief 动态字符串结构体
 */
typedef struct ek_str_t ek_str_t;

struct ek_str_t
{
    char *buf; /**< 字符串缓冲区指针 */
    uint32_t cap; /**< 缓冲区容量（字节数） */
    uint32_t len; /**< 当前字符串长度（不包含 \0） */
};

/**
 * @brief 创建动态字符串
 * @param str 初始字符串内容（可为 NULL，表示创建空字符串）
 * @return 字符串对象指针，失败返回 NULL
 * @note 使用完毕后需调用 ek_str_free() 释放
 */
ek_str_t *ek_str_create(const char *str);

/**
 * @brief 释放动态字符串
 * @param s 字符串对象指针
 */
void ek_str_free(ek_str_t *s);

/**
 * @brief 清空字符串内容
 * @param s 字符串对象指针
 * @note 不释放内存，仅将长度置为 0
 */
void ek_str_clear(ek_str_t *s);

/**
 * @brief 追加指定长度的字符串
 * @param s 字符串对象指针
 * @param str 要追加的字符串
 * @param len 追加的长度
 * @return true 成功，false 失败（内存不足）
 */
bool ek_str_append_len(ek_str_t *s, const char *str, uint32_t len);

/**
 * @brief 追加字符串
 * @param s 字符串对象指针
 * @param str 要追加的字符串（以 \0 结尾）
 * @return true 成功，false 失败（内存不足）
 */
bool ek_str_append(ek_str_t *s, const char *str);

/**
 * @brief 追加格式化字符串
 * @param s 字符串对象指针
 * @param fmt 格式化字符串
 * @param ... 可变参数
 * @return true 成功，false 失败（内存不足）
 */
bool ek_str_append_fmt(ek_str_t *s, const char *fmt, ...);

/**
 * @brief 拼接两个字符串
 * @param dst 目标字符串
 * @param src 源字符串
 * @return true 成功，false 失败（内存不足）
 * @note 将 src 的内容追加到 dst 末尾
 */
bool ek_str_cat(ek_str_t *dst, ek_str_t *src);

/**
 * @brief 字符串切片
 * @param s 源字符串
 * @param start 起始索引（支持负数，-1 表示最后一个字符）
 * @param end 结束索引（不包含，支持负数）
 * @return 新的字符串对象，失败返回 NULL
 * @note 返回的字符串需要调用 ek_str_free() 释放
 */
ek_str_t *ek_str_slice(const ek_str_t *s, int32_t start, int32_t end);

/**
 * @brief 获取 C 风格字符串
 * @param s 字符串对象指针
 * @return C 风格字符串指针（以 \0 结尾）
 * @note 返回的指针指向内部缓冲区，不要手动释放
 */
const char *ek_str_get_cstring(ek_str_t *s);

/**
 * @brief 获取字符串长度
 * @param s 字符串对象指针
 * @return 字符串长度（不包含 \0）
 */
uint32_t ek_str_get_len(ek_str_t *s);

/**
 * @brief 获取缓冲区容量
 * @param s 字符串对象指针
 * @return 缓冲区容量（字节数）
 */
uint32_t ek_str_get_cap(ek_str_t *s);

/**
 * @brief 比较两个字符串
 * @param s1 字符串 1
 * @param s2 字符串 2
 * @return 0 相等，< 0 s1 < s2，> 0 s1 > s2
 */
int ek_str_cmp(ek_str_t *s1, ek_str_t *s2);

/**
 * @brief 比较两个字符串的前 n 个字符
 * @param s1 字符串 1
 * @param s2 字符串 2
 * @param n 比较的字符数
 * @return 0 相等，< 0 s1 < s2，> 0 s1 > s2
 */
int ek_str_ncmp(ek_str_t *s1, ek_str_t *s2, size_t n);

#    ifdef __cplusplus
}
#    endif

#endif // EK_STR_ENABLE

#endif // EK_STR_H
