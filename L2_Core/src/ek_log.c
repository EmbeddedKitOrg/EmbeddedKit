/* ========================= Includes ========================= */
#include "../inc/ek_log.h"

/* ========================= Internal Macros ========================= */
#define EK_LOG_COLOR_NONE   "\033[0;0m"
#define EK_LOG_COLOR_YELLOW "\033[33m"
#define EK_LOG_COLOR_RED    "\033[91m"
#define EK_LOG_COLOR_BLUE   "\033[94m"
#define EK_LOG_COLOR_GREEN  "\033[92m"

#define EK_LOG_CHECK_LOCK() (_lock == 1)
#define EK_LOG_LOCK()       (_lock = 1)
#define EK_LOG_UNLOCK()     (_lock = 0)

/* ========================= Global Variables ========================= */
static uint8_t _lock = 0;

#if (EK_LOG_COLOR_ENABLE == 1)

static const char *const ek_log_color_table[EK_LOG_MAX] = {
    EK_LOG_COLOR_NONE, // EK_LOG_NONE
    EK_LOG_COLOR_GREEN, // EK_LOG_DEBUG
    EK_LOG_COLOR_BLUE, // EK_LOG_INFO
    EK_LOG_COLOR_YELLOW, // EK_LOG_WARN
    EK_LOG_COLOR_RED, // EK_LOG_ERROR
};

#endif /* (EK_LOG_COLOR_ENABLE == 1) */

static const char *ek_log_type_table[EK_LOG_MAX] = {
    "None", // EK_LOG_NONE
    "Debug", // EK_LOG_DEBUG
    "Info", // EK_LOG_INFO
    "Warn", // EK_LOG_WARN
    "Error" // EK_LOG_ERROR
};

static char ek_log_buffer[EK_LOG_MAX_BUFFER];

/* ========================= Internal Functions ========================= */
uint32_t common_get_tick_ms()
{
    return 0;
}
/* ========================= Public Functions ========================= */
/**
 * @brief 日志打印
 * 
 * @param tag 文件标签
 * @param line 行号
 * @param type 日志类型
 * @param fmt 格式化
 * @param ... 
 */
void _ek_log_printf(const char *tag, uint32_t line, ek_log_type_t type, const char *fmt, ...)
{
    if (EK_LOG_CHECK_LOCK() == 1) return;

    EK_LOG_LOCK();

#if (EK_LOG_COLOR_ENABLE == 1)

    lwprintf("%s[%s/%s L:%" PRIu32 ",T:%" PRIu32 "]:",
             ek_log_color_table[type],
             ek_log_type_table[type],
             tag,
             line,
             common_get_tick_ms());
#else /* EK_LOG_COLOR_ENABLE == 1 */
    lwprintf("[%s/%s L:%" PRIu32 ",T:%" PRIu32 "]:", ek_log_type_table[type], tag, line, common_get_tick_ms());
#endif /* EK_LOG_COLOR_ENABLE == 1 */

    va_list args;
    va_start(args, fmt);
    uint32_t length = lwvsnprintf(ek_log_buffer, EK_LOG_MAX_BUFFER - 1, fmt, args);
    va_end(args);
    ek_log_buffer[length] = '\0';

    lwprintf("%s", ek_log_buffer);

#if (EK_LOG_COLOR_ENABLE == 1)
    lwprintf(EK_LOG_COLOR_NONE); // 恢复日志颜色
#endif /* EK_LOG_COLOR_ENABLE == 1 */

    lwprintf(CRLF); // 换行符

    EK_LOG_UNLOCK();
}
