/**
 * @file ek_log.h
 * @brief 日志系统
 * @author N1netyNine99
 *
 * 提供分级日志输出功能，支持：
 * - 多级别日志（DEBUG、INFO、WARN、ERROR）
 * - 文件名、行号、时间戳输出
 * - 彩色打印（可选）
 * - 格式化输出
 *
 * @note 使用前需实现 _ek_log_get_tick() 函数，用于获取系统时间戳
 */

#ifndef EK_LOG_H
#define EK_LOG_H

#include "../../../ek_conf.h"

#if EK_LOG_ENABLE == 1

#    include "ek_io.h"
#    include "ek_def.h"

/**
 * @brief 是否启用彩色打印
 * @note 1 = 启用 ANSI 颜色代码，0 = 纯文本输出
 */
#    ifndef EK_LOG_COLOR_ENABLE
#        define EK_LOG_COLOR_ENABLE (0)
#    endif /* EK_LOG_COLOR_ENABLE   */

/**
 * @brief 是否启用 DEBUG 级别日志
 * @note 1 = 启用，0 = 禁用（可减小代码体积）
 */
#    ifndef EK_LOG_DEBUG_ENABLE
#        define EK_LOG_DEBUG_ENABLE (1)
#    endif /* EK_LOG_DEBUG_ENABLE   */

/**
 * @brief 日志缓冲区大小
 * @note 单条日志的最大长度（字节）
 */
#    ifndef EK_LOG_BUFFER_SIZE
#        define EK_LOG_BUFFER_SIZE (256)
#    endif /* EK_LOG_BUFFER_SIZE     */

/**
 * @brief 定义文件标签
 * @param tag 标签字符串
 * @note 在源文件开头使用，用于标识日志来源
 * @example
 * EK_LOG_FILE_TAG("main.c");
 */
#    define EK_LOG_FILE_TAG(tag) static const char *_EK_LOG_TAG_ = tag;

/**
 * @brief 定义获取时间戳函数
 * @note 用户需要实现此函数，返回系统时间戳（单位：毫秒）
 * @example
 * EK_LOG_GET_TICK()
 * {
 *     return HAL_GetTick();
 * }
 */
#    define EK_LOG_GET_TICK() uint32_t _ek_log_get_tick(void)

/**
 * @brief 日志级别枚举
 */
typedef enum
{
    EK_LOG_TYPE_NONE = 0, /**< 无级别（普通输出） */
    EK_LOG_TYPE_DEBUG, /**< 调试信息 */
    EK_LOG_TYPE_INFO, /**< 一般信息 */
    EK_LOG_TYPE_WARN, /**< 警告信息 */
    EK_LOG_TYPE_ERROR, /**< 错误信息 */

    EK_LOG_TYPE_MAX = 5, /**< 最大级别数 */
} ek_log_type_t;

#    ifdef __cplusplus
extern "C"
{
#    endif

/**
 * @brief 日志输出函数（内部使用）
 * @param tag 文件标签
 * @param line 行号
 * @param type 日志级别
 * @param tick 时间戳
 * @param fmt 格式化字符串
 * @param ... 可变参数
 */
void _ek_log_printf(const char *tag, uint32_t line, ek_log_type_t type, uint32_t tick, const char *fmt, ...);

/**
 * @brief 获取系统时间戳（用户实现）
 * @return 时间戳（单位：毫秒）
 */
uint32_t _ek_log_get_tick(void);

/**
 * @brief DEBUG 级别日志
 * @param ... 格式化字符串和参数
 * @note 仅在 EK_LOG_DEBUG_ENABLE == 1 时有效
 */
#    if (EK_LOG_DEBUG_ENABLE == 1)
#        define EK_LOG_DEBUG(...) \
            _ek_log_printf(_EK_LOG_TAG_, __LINE__, EK_LOG_TYPE_DEBUG, _ek_log_get_tick(), __VA_ARGS__)
#    else
#        define EK_LOG_DEBUG(...)
#    endif

/**
 * @brief 普通日志（无级别标识）
 * @param ... 格式化字符串和参数
 */
#    define EK_LOG(...) _ek_log_printf(_EK_LOG_TAG_, __LINE__, EK_LOG_TYPE_NONE, _ek_log_get_tick(), __VA_ARGS__)

/**
 * @brief INFO 级别日志
 * @param ... 格式化字符串和参数
 */
#    define EK_LOG_INFO(...) _ek_log_printf(_EK_LOG_TAG_, __LINE__, EK_LOG_TYPE_INFO, _ek_log_get_tick(), __VA_ARGS__)

/**
 * @brief WARN 级别日志
 * @param ... 格式化字符串和参数
 */
#    define EK_LOG_WARN(...) _ek_log_printf(_EK_LOG_TAG_, __LINE__, EK_LOG_TYPE_WARN, _ek_log_get_tick(), __VA_ARGS__)

/**
 * @brief ERROR 级别日志
 * @param ... 格式化字符串和参数
 */
#    define EK_LOG_ERROR(...) _ek_log_printf(_EK_LOG_TAG_, __LINE__, EK_LOG_TYPE_ERROR, _ek_log_get_tick(), __VA_ARGS__)

#    ifdef __cplusplus
}
#    endif

#endif /* EK_LOG_ENABLE */

#endif // __SER_LOG_H
