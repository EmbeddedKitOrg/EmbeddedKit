/**
 * @file shell_symbols.c
 * @brief 为非嵌入式平台的letter_shell提供链接器符号和内存管理函数
 * 
 * 在嵌入式系统中，这些符号通常由链接脚本提供。
 * 在Windows/Linux上进行测试时，我们需要显式定义它们。
 * 
 * 同时，在Windows下weak符号支持有限，所以直接提供内存管理函数的实现。
 */

#include <stdlib.h>
#include "../L2_Core/third_party/letter_shell/inc/shell.h"

/* 为测试环境提供shell命令段符号（空段） */
#if defined(__GNUC__) && !defined(__ARMCC_VERSION)

/* 为Windows/Linux上的GCC提供通常由链接脚本提供的符号 */
const unsigned int _shell_command_start __attribute__((weak)) = 0;
const unsigned int _shell_command_end __attribute__((weak)) = 0;

#    if SHELL_USING_VAR == 1
const unsigned int _shell_variable_start __attribute__((weak)) = 0;
const unsigned int _shell_variable_end __attribute__((weak)) = 0;
#    endif

#endif

/* 为测试环境提供内存管理函数（直接使用标准库） */
#if defined(_WIN32) || defined(_WIN64)

/* Windows下提供强符号，避免weak符号链接问题 */
void *_ek_malloc(size_t size)
{
    return malloc(size);
}

void *_ek_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

void _ek_free(void *ptr)
{
    free(ptr);
}

#endif
