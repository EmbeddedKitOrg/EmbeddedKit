// /**
//  * @file shell_symbols.c
//  * @brief 为非嵌入式平台的letter_shell提供链接器符号
//  * 
//  * 在嵌入式系统中，这些符号通常由链接脚本提供。
//  * 在Windows/Linux上进行测试时，我们需要显式定义它们。
//  */

// #include "../L2_Core/third_party/letter_shell/inc/shell.h"

// /* 为测试定义空的命令和变量段 */
// #if defined(__GNUC__) && !defined(__ARMCC_VERSION)

// /* 为Windows/Linux上的GCC提供通常由链接脚本提供的符号 */
// const unsigned int _shell_command_start __attribute__((weak)) = 0;
// const unsigned int _shell_command_end __attribute__((weak)) = 0;

// #    if SHELL_USING_VAR == 1
// const unsigned int _shell_variable_start __attribute__((weak)) = 0;
// const unsigned int _shell_variable_end __attribute__((weak)) = 0;
// #    endif

// #endif
