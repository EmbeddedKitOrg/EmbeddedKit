/**
 * @file ek_ringbuf.h
 * @brief 环形缓冲区（Ring Buffer）
 *
 * 通用的环形缓冲区实现，支持任意数据类型的存储
 */

#ifndef EK_RINGBUF_H
#define EK_RINGBUF_H

#include "../../../ek_conf.h"

#if EK_RINGBUF_ENABLE == 1

#    include "ek_def.h"


/**
 * @brief 环形缓冲区结构
 */
typedef struct ek_ringbuf_t ek_ringbuf_t;

struct ek_ringbuf_t
{
    uint8_t *buffer; /**< 缓冲区指针 */
    uint32_t write_idx; /**< 写入位置索引 */
    uint32_t read_idx; /**< 读取位置索引 */
    uint32_t item_amount; /**< 当前元素个数 */
    size_t cap; /**< 缓冲区容量（元素个数） */
    size_t item_size; /**< 单个元素大小（字节） */
#    if EK_USE_RTOS == 1
    bool lock;
#    endif /* EK_USE_RTOS */
};

#    ifdef __cplusplus
extern "C"
{
#    endif /* __cplusplus */

/**
 * @brief 判断环形缓冲区是否已满
 * @param rb 环形缓冲区指针
 * @return true 已满
 * @return false 未满
 */
bool ek_ringbuf_full(const ek_ringbuf_t *rb);

/**
 * @brief 判断环形缓冲区是否为空
 * @param rb 环形缓冲区指针
 * @return true 为空
 * @return false 不为空
 */
bool ek_ringbuf_empty(const ek_ringbuf_t *rb);

/**
 * @brief 创建环形缓冲区
 * @param item_size 单个元素大小（字节）
 * @param item_amount 缓冲区容量（元素个数）
 * @return 成功返回缓冲区指针，失败返回 NULL
 */
ek_ringbuf_t *ek_ringbuf_create(size_t item_size, uint32_t item_amount);

/**
 * @brief 销毁环形缓冲区
 * @param rb 要销毁的环形缓冲区
 *
 * @note ek_free 会自动将 rb->buffer 置为 NULL，但 rb 本身需要调用者手动置空
 */
void ek_ringbuf_destroy(ek_ringbuf_t *rb);

/**
 * @brief 销毁环形缓冲区并把rb_ptr设置为NULL
 * @param rb_ptr 要销毁的环形缓冲区
 *
 */
#    define ek_ringbuf_destroy_safely(rb_ptr) \
        do                                    \
        {                                     \
            ek_ringbuf_destroy(rb_ptr);       \
            rb_ptr = NULL;                    \
        } while (0)

/**
 * @brief 向环形缓冲区写入一个元素
 * @param rb 环形缓冲区指针
 * @param item 要写入的元素指针
 * @return true 写入成功
 * @return false 写入失败（缓冲区已满）
 */
bool ek_ringbuf_write(ek_ringbuf_t *rb, const void *item);

/**
 * @brief 从环形缓冲区读取一个元素
 * @param rb 环形缓冲区指针
 * @param item 存储读取结果的缓冲区指针，传入 NULL 则直接丢弃数据
 * @return true 读取成功
 * @return false 读取失败（缓冲区为空）
 */
bool ek_ringbuf_read(ek_ringbuf_t *rb, void *item);

/**
 * @brief 查看环形缓冲区首个元素（不移动读指针）
 * @param rb 环形缓冲区指针
 * @param item 存储查看结果的缓冲区指针
 * @return true 查看成功
 * @return false 查看失败（缓冲区为空）
 */
bool ek_ringbuf_peek(ek_ringbuf_t *rb, void *item);

#    ifdef __cplusplus
}
#    endif /* __cplusplus */

#endif /* EK_RINGBUF_ENABLE */

#endif /* EK_RINGBUF_H */
