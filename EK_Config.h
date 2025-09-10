/**
 * @file EK_Config.h
 * @brief EmbeddedKit 配置文件
 * @details 包含系统配置参数和编译选项
 * @author N1ntyNine99
 * @version 1.0
 * @date 2025-01-09
 */

#ifndef __EK_CONFIG_H
#define __EK_CONFIG_H

#include "EK_Common.h"

/**
 * @brief 内存分配宏定义
 * @details 函数签名要求是 void* xxx(size_t)
 */
#define _MALLOC(x) malloc(x)

/**
 * @brief 内存释放宏定义
 * @details 函数入参要求是 void*
 */
#define _FREE(X) free(X)

/**
 * @brief 内存池总大小 (字节)
 * @note 可根据系统资源调整，建议至少1KB
 */
#define MEMPOOL_SIZE (4096)

/**
 * @brief 内存对齐大小 (字节)
 * @note 必须是2的幂次，通常为4或8字节
 */
#define MEMPOOL_ALIGNMENT (8)

/**
 * @brief 链表相关，是否启用递归排序 0:不启用 1:启用
 * @note 启用递归排序可以提高排序效率，但可能导致栈溢出
 * @note 禁用递归排序可以避免栈溢出，但可能导致排序效率下降
 */
#define LIST_RECURSION_SORT (1)

#endif
