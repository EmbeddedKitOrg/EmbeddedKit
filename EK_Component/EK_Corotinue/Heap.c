/**
 * @file Heap.c
 * @brief 协程内存管理模块
 * @author N1ntyNine99
 * @version 1.0
 * @date 2025-10-07
 *
 * @details
 * 本文件提供协程系统的线程安全内存分配和释放函数。
 * 用户可以根据实际需求选择合适的内存分配实现方式。
 *
 * @note
 * 这是示例实现，使用临界区保护内存分配操作。
 * 在实际项目中，用户可以选择：
 * - 使用临界区保护的简单内存分配器（如本示例）
 * - 使用内存池分配器以提高确定性和避免碎片
 * - 使用更复杂的内存管理算法
 * - 直接使用系统提供的内存分配函数
 */

#include "./Inc/Kernel.h"

/**
 * @brief 协程内存分配函数
 *
 * @param size 需要分配的内存大小（字节）
 * @return void* 分配成功的内存指针，失败时返回NULL
 *
 * @details
 * 此函数为协程系统提供线程安全的内存分配功能。
 * 使用临界区保护内存分配操作，确保在多任务环境下的安全性。
 *
 * @note
 * 用户可以根据实际需求自定义此函数的实现：
 * 1. 使用系统malloc（如本示例）
 * 2. 使用内存池分配器（推荐嵌入式系统）
 * 3. 使用其他内存管理算法
 * 4. 添加内存使用统计和调试功能
 *
 * @warning
 * - 调用此函数前必须确保EK_MALLOC已正确定义(正确包含了 `EK_Config.h` 头文件)
 * - 返回的内存必须通过EK_Coro_Free释放
 * - 分配失败时应检查返回值是否为NULL
 */
void *EK_Coro_Malloc(EK_Size_t size)
{
    void *ptr = NULL;
    EK_ENTER_CRITICAL();
    ptr = EK_MALLOC(size);
    EK_EXIT_CRITICAL();
    return ptr;
}

/**
 * @brief 协程内存释放函数
 *
 * @param ptr 需要释放的内存指针（必须是通过EK_Coro_Malloc分配的内存）
 * @return void
 *
 * @details
 * 此函数为协程系统提供线程安全的内存释放功能。
 * 使用临界区保护内存释放操作，确保在多任务环境下的安全性。
 *
 * @note
 * 用户可以根据实际需求自定义此函数的实现：
 * 1. 使用系统free（如本示例）
 * 2. 使用内存池释放器（推荐嵌入式系统）
 * 3. 添加内存泄漏检测功能
 * 4. 添加内存使用统计和调试功能
 *
 * @warning
 * - 传入的指针必须是通过EK_Coro_Malloc分配的(正确包含了 `EK_Config.h` 头文件)
 * - 不要重复释放同一块内存
 * - 不要释放NULL指针（虽然大多数free实现会忽略）
 * - 释放后应将指针设为NULL以避免悬垂指针
 */
void EK_Coro_Free(void *ptr)
{
    EK_ENTER_CRITICAL();
    EK_FREE(ptr);
    EK_EXIT_CRITICAL();
}