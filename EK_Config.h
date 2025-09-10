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

#ifdef __cplusplus
extern "C"{
#endif

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


#ifdef __cplusplus
}
#endif


#endif
