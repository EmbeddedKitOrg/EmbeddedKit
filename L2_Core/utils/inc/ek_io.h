/**
 * @file ek_io.h
 * @brief 标准输入输出接口
 * @author N1netyNine99
 *
 * 提供基于 lwprintf 的轻量级标准输入输出功能
 *
 * @note 使用前需实现 _ek_io_fputc() 函数，用于底层字符输出
 */

#ifndef EK_IO_H
#define EK_IO_H

#include "../../../ek_conf.h"

#if EK_IO_ENABLE == 1

#    include "../../third_party/lwprintf/inc/lwprintf.h"

/**
 * @brief 定义字符输出函数
 * @note 用户需要实现此函数，用于底层字符输出（如 UART）
 * @example
 * EK_IO_FPUTC()
 * {
 *     // 将字符 ch 输出到 UART
 *     HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, 100);
 * }
 */
#    define EK_IO_FPUTC() void _ek_io_fputc(int ch)

/**
 * @brief 初始化 I/O 系统
 * @note 初始化 lwprintf 库，设置输出函数
 */
void ek_io_init(void);

#endif /* EK_IO_ENABLE */

#endif
