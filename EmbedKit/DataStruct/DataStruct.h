/**
 * @file DataStruct.h
 * @brief 数据结构模块通用头文件
 * @details 定义了数据结构模块的通用类型、宏定义和函数声明
 * @author N1ntyNine99
 * @date 2025-09-08
 * @version 1.0
 */

#ifndef __DATASTRUCT_H
#define __DATASTRUCT_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 数据结构操作结果枚举
 * @details 定义了数据结构操作的通用返回码
 */
typedef enum
{
    DS_SUCCESS = 0, /**< 操作成功 */
    DS_ERROR_NULL_POINTER, /**< 空指针错误 */
    DS_ERROR_INVALID_PARAM, /**< 无效参数 */
    DS_ERROR_MEMORY_ALLOC, /**< 内存分配失败 */
    DS_ERROR_EMPTY, /**< 数据结构为空 */
    DS_ERROR_FULL, /**< 数据结构已满 */
    DS_ERROR_NOT_FOUND, /**< 未找到目标 */
    DS_ERROR_INSUFFICIENT_SPACE, /**< 空间不足 */
    DS_ERROR_UNKNOWN /**< 未知错误 */
} DS_Result_t;

typedef size_t index_t;

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

#ifdef __cplusplus
}
#endif

#endif
