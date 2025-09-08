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
