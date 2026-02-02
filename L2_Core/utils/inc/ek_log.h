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

#    define EK_LOG_FILE_TAG(tag) static const char *_EK_LOG_TAG_ = tag;

#    define EK_LOG_GET_TICK()    uint32_t _ek_log_get_tick(void)

typedef enum
{
    EK_LOG_TYPE_NONE = 0,
    EK_LOG_TYPE_DEBUG,
    EK_LOG_TYPE_INFO,
    EK_LOG_TYPE_WARN,
    EK_LOG_TYPE_ERROR,

    EK_LOG_TYPE_MAX = 5,
} ek_log_type_t;

#    ifdef __cplusplus
extern "C"
{
#    endif

void _ek_log_printf(const char *tag, uint32_t line, ek_log_type_t type, uint32_t tick, const char *fmt, ...);
uint32_t _ek_log_get_tick(void);

#    if (EK_LOG_DEBUG_ENABLE == 1)
#        define EK_LOG_DEBUG(...) \
            _ek_log_printf(_EK_LOG_TAG_, __LINE__, EK_LOG_TYPE_DEBUG, _ek_log_get_tick(), __VA_ARGS__)
#    else
#        define EK_LOG_DEBUG(...)
#    endif

#    define EK_LOG(...)       _ek_log_printf(_EK_LOG_TAG_, __LINE__, EK_LOG_TYPE_NONE, _ek_log_get_tick(), __VA_ARGS__)
#    define EK_LOG_INFO(...)  _ek_log_printf(_EK_LOG_TAG_, __LINE__, EK_LOG_TYPE_INFO, _ek_log_get_tick(), __VA_ARGS__)
#    define EK_LOG_WARN(...)  _ek_log_printf(_EK_LOG_TAG_, __LINE__, EK_LOG_TYPE_WARN, _ek_log_get_tick(), __VA_ARGS__)
#    define EK_LOG_ERROR(...) _ek_log_printf(_EK_LOG_TAG_, __LINE__, EK_LOG_TYPE_ERROR, _ek_log_get_tick(), __VA_ARGS__)

#    ifdef __cplusplus
}
#    endif

#endif /* EK_LOG_ENABLE */

#endif // __SER_LOG_H
