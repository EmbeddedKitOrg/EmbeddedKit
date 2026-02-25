/**
 * @file ek_assert.c
 * @brief 断言机制实现
 * @author N1netyNine99
 */

#include "../inc/ek_assert.h"
#include "../inc/ek_log.h"

void ek_assert_fault(const char *file, uint32_t line, const char *expr)
{
#if EK_ASSERT_WITH_LOG == 1
    EK_LOG_FILE_TAG("ek_assert.c");
    EK_LOG_ERROR("file:%s,line:%" PRIu32 ",expr: %s", EK_GET_FILE_NAME(file), line, expr);
#else
    __UNUSED(file);
    __UNUSED(line);
    __UNUSED(expr);
#endif /* EK_ASSERT_WITH_LOG */

    while (1)
    {
    }
}
