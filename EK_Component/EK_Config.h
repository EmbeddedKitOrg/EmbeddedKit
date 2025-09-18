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

extern void *EK_pMemPool_Malloc(EK_Size_t size);
extern void EK_vMemPool_FreeSafely(void *ptr);

/**
 * @brief 内存分配宏定义
 * @details 函数签名要求是 void* xxx(EK_Size_t)
 */
#define EK_MALLOC(X) EK_pMemPool_Malloc(X)

/**
 * @brief 内存释放宏定义
 * @details 函数入参要求是 void*
 */
#define EK_FREE(X) EK_vMemPool_FreeSafely(X)

/**
 * @brief 内存池总大小 (字节)
 * @note 可根据系统资源调整，建议至少1KB
 */
#define MEMPOOL_SIZE (10240)

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

/**
 * @brief 通讯相关 发送缓冲区大小
 * 
 */
#define SERIAL_TX_BUFFER (256)

/**
 * @brief 每次轮询发送的最大字节数
 * @details 限制单次发送数据量，确保消息顺序和实时性
 */
#define SERIAL_MAX_SEND_SIZE (128)

/**
 * @brief 队列满时的处理策略
 * @details 0: 直接丢弃新数据; 1: 丢弃最老的数据腾出空间
 */
#define SERIAL_FULL_STRATEGY (1)

/**
 * @brief 串口轮询发送的默认间隔时间（单位：ms）
 * @details 当串口队列中有数据时，每隔这么久发送一次数据。
 */
#define SERIAL_OVER_TIME (20)

/**
 * @brief 通讯相关 遍历间隔时间(ms)
 */
#define SERIAL_POLL_INTERVAL (5)

#endif
