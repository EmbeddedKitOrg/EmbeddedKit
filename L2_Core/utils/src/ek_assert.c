#include "../inc/ek_assert.h"
#include "../inc/ek_log.h"
#include <string.h>

#define GET_FILE_NAME(file_path)                           \
    (strrchr(file_path, '/') ? strrchr(file_path, '/') + 1 \
                             : (strrchr(file_path, '\\') ? strrchr(file_path, '\\') + 1 : file_path))

void ek_assert_fault(const char *file, uint32_t line, const char *expr)
{
#if EK_ASSERT_WITH_LOG == 1
    EK_LOG_FILE_TAG("ek_assert.c");
    EK_LOG_ERROR("file:%s,line:%" PRIu32 ",expr: %s", GET_FILE_NAME(file), line, expr);
#else
    __UNUSED(file);
    __UNUSED(line);
    __UNUSED(expr);
#endif /* EK_ASSERT_WITH_LOG */

    while (1)
    {
    }
}
