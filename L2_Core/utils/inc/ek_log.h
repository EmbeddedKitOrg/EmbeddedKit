#ifndef EK_LOG_H
#define EK_LOG_H

#include "../../../ek_conf.h"

#if EK_LOG_ENABLE == 1

#    include "ek_io.h"
#    include "ek_def.h"

#    ifndef EK_LOG_COLOR_ENABLE
#        define EK_LOG_COLOR_ENABLE (0) // 使能彩色打印
#    endif /* EK_LOG_COLOR_ENABLE   */

#    ifndef EK_LOG_DEBUG_ENABLE
#        define EK_LOG_DEBUG_ENABLE (1) // 是否开启debug
#    endif /* EK_LOG_DEBUG_ENABLE   */

#    ifndef EK_LOG_BUFFER_SIZE
#        define EK_LOG_BUFFER_SIZE (256) // 字符串缓冲区
#    endif /* EK_LOG_BUFFER_SIZE     */

#    define EK_LOG_FILE_TAG(tag) static const char *__EK_LOG_TAG__ = tag;

EK_ENUM(ek_log_type_t, uint8_t){
    EK_LOG_TYPE_NONE = 0,
    EK_LOG_TYPE_DEBUG,
    EK_LOG_TYPE_INFO,
    EK_LOG_TYPE_WARN,
    EK_LOG_TYPE_ERROR,

    EK_LOG_TYPE_MAX = 5,
};

#    ifdef __cplusplus
extern "C"
{
#    endif

/**
 * @brief 日志打印
 * 
 * @param tag 文件标签
 * @param line 行号
 * @param type 日志类型
 * @param fmt 格式化
 * @param ... 
 */
void _ek_log_printf(const char *tag, uint32_t line, ek_log_type_t type, const char *fmt, ...);

#    if (EK_LOG_DEBUG_ENABLE == 1)
#        define EK_LOG_DEBUG(...) _ek_log_printf(__EK_LOG_TAG__, __LINE__, EK_LOG_TYPE_DEBUG, __VA_ARGS__)
#    else
#        define EK_LOG_DEBUG(...)
#    endif

#    define EK_LOG(...)       _ek_log_printf(__EK_LOG_TAG__, __LINE__, EK_LOG_TYPE_NONE, __VA_ARGS__)
#    define EK_LOG_INFO(...)  _ek_log_printf(__EK_LOG_TAG__, __LINE__, EK_LOG_TYPE_INFO, __VA_ARGS__)
#    define EK_LOG_WARN(...)  _ek_log_printf(__EK_LOG_TAG__, __LINE__, EK_LOG_TYPE_WARN, __VA_ARGS__)
#    define EK_LOG_ERROR(...) _ek_log_printf(__EK_LOG_TAG__, __LINE__, EK_LOG_TYPE_ERROR, __VA_ARGS__)

#    ifdef __cplusplus
}
#    endif

#endif /* EK_LOG_ENABLE */

#endif // __SER_LOG_H
