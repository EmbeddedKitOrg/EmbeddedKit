/**
 * @file ek_stack.h
 * @brief 通用栈数据结构
 *
 * 本模块实现了一个类型通用的栈（后进先出，LIFO）数据结构。
 * 支持任意数据类型的存储，通过指定元素大小实现类型安全。
 *
 * @note 栈基于动态内存分配，使用前需调用 ek_stack_create() 创建，
 *       使用完毕后需调用 ek_stack_destroy() 释放内存。
 *
 * @warning 所有操作函数均会对传入的栈指针进行断言检查，
 *          传入 NULL 指针将在调试模式下触发断言失败。
 */

#ifndef EK_STACK_H
#define EK_STACK_H

#include "ek_def.h"

/**
 * @brief 栈结构体
 *
 * 封装了栈的基本属性，包括缓冲区指针、栈顶指针、元素大小和容量。
 */
typedef struct ek_stack_t ek_stack_t;

struct ek_stack_t
{
    void *buffer; /**< 数据缓冲区指针 */
    uint32_t sp; /**< 栈顶指针（stack pointer），指向下一个写入位置 */
    size_t item_size; /**< 单个元素的大小（字节） */
    uint32_t cap; /**< 栈的最大容量 */
};

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 * @brief 检查栈是否已满
 *
 * @param sk 栈指针
 * @return true 栈已满
 * @return false 栈未满
 *
 * @note 复杂度：O(1)
 */
bool ek_stack_full(ek_stack_t *sk);

/**
 * @brief 检查栈是否为空
 *
 * @param sk 栈指针
 * @return true 栈为空
 * @return false 栈非空
 *
 * @note 复杂度：O(1)
 */
bool ek_stack_empty(ek_stack_t *sk);

/**
 * @brief 创建一个新栈
 *
 * 分配内存并初始化一个栈实例。
 *
 * @param item_size 单个元素的大小（字节）
 * @param item_amount 栈的容量（最多可存储的元素数量）
 * @return 成功返回栈指针，失败返回 NULL
 *
 * @warning item_size 和 item_amount 不能为 0，否则触发断言
 * @warning 内存分配失败时返回 NULL，调用者需检查返回值
 * @note 失败时会自动释放已分配的内存，不会造成内存泄漏
 */
ek_stack_t *ek_stack_create(size_t item_size, uint32_t item_amount);

/**
 * @brief 销毁栈并释放内存
 *
 * 释放栈及其内部缓冲区占用的所有内存。
 *
 * @param sk 栈指针
 *
 * @note ek_free 会自动将 sk->buffer 置为 NULL，但 sk 本身需要调用者手动置空
 * @warning 传入 NULL 指针将触发断言失败
 * @warning 不应对同一个栈多次调用此函数
 */
void ek_stack_destroy(ek_stack_t *sk);

/**
 * @brief 销毁栈并把sk_ptr设置为NULL
 * @param sk_ptr 要销毁的栈指针
 */
#define ek_stack_destroy_safely(sk_ptr) \
    do                                  \
    {                                   \
        ek_stack_destroy(sk_ptr);       \
        sk_ptr = NULL;                  \
    } while (0)

/**
 * @brief 将元素压入栈
 *
 * 将一个元素复制到栈顶。
 *
 * @param sk 栈指针
 * @param item 指向要压入的元素的指针
 * @return true 成功压入
 * @return false 栈已满，压入失败
 *
 * @warning sk 和 item 不能为 NULL，否则触发断言
 * @note 复杂度：O(1)
 * @note 会按 item_size 字节数复制数据
 */
bool ek_stack_push(ek_stack_t *sk, const void *item);

/**
 * @brief 从栈顶弹出一个元素
 *
 * 将栈顶元素复制到指定位置，并从栈中移除。
 *
 * @param sk 栈指针
 * @param item 指向接收弹出元素的缓冲区的指针
 * @return true 成功弹出
 * @return false 栈为空，弹出失败
 *
 * @warning sk 和 item 不能为 NULL，否则触发断言
 * @note 复杂度：O(1)
 * @note 会按 item_size 字节数复制数据
 */
bool ek_stack_pop(ek_stack_t *sk, void *item);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* EK_STACK_H */
