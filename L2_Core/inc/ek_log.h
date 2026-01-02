#ifndef EK_LOG_H
#define EK_LOG_H

#include "ek_io.h"
#include "ek_def.h"

#define EK_LOG_COLOR_ENABLE (1) // 使能彩色打印
#define EK_LOG_DEBUG_ENABLE (1) // 是否开启debug
#define EK_LOG_MAX_BUFFER   (256) // 字符串缓冲区

EK_ENUM(ek_log_type_t, uint8_t){
    EK_LOG_NONE = 0,
    EK_LOG_DEBUG,
    EK_LOG_INFO,
    EK_LOG_WARN,
    EK_LOG_ERROR,

    EK_LOG_MAX = 5,
};

#ifdef __cplusplus
extern "C"
{
#endif
void _ek_log_printf(const char *tag, uint32_t line, ek_log_type_t type, const char *fmt, ...);

#if (EK_LOG_DEBUG_ENABLE == 1)
#    define ser_log_debug(...) _ek_log_printf(__FILE__, __LINE__, EK_LOG_DEBUG, __VA_ARGS__)
#else
#    define ser_log_debug(...)
#endif

#define ser_log(...)       _ek_log_printf(__FILE__, __LINE__, EK_LOG_NONE, __VA_ARGS__)
#define ser_log_info(...)  _ek_log_printf(__FILE__, __LINE__, EK_LOG_INFO, __VA_ARGS__)
#define ser_log_warn(...)  _ek_log_printf(__FILE__, __LINE__, EK_LOG_WARN, __VA_ARGS__)
#define ser_log_error(...) _ek_log_printf(__FILE__, __LINE__, EK_LOG_ERROR, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // __SER_LOG_H
