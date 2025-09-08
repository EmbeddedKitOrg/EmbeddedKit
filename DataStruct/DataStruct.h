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
