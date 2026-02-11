/**
 * @file ek_shell.h
 * @brief Shell 命令行接口
 * @author N1netyNine99
 *
 * 提供基于 letter_shell 的命令行交互功能
 *
 * @note 使用前需在链接脚本 (.ld) 中添加以下段定义：
 * @code
 * _shell_command_start = .;
 * KEEP (*(shellCommand))
 * _shell_command_end = .;
 * @endcode
 */

#ifndef EK_SHELL_H
#define EK_SHELL_H

#include "ek_conf.h"

#if EK_SHELL_ENABLE == 1

#include "../../third_party/letter_shell/inc/shell.h"

/**
 * @brief 导出整型变量到 Shell
 * @param var Shell 中的变量名
 * @param variable C 代码中的变量名
 * @param desc 变量描述
 */
#define EK_SHELL_EXPORT_VAR_INT(var, variable, desc)   SHELL_EXPORT_VAR_INT(var, variable, desc)

/**
 * @brief 导出短整型变量到 Shell
 * @param var Shell 中的变量名
 * @param variable C 代码中的变量名
 * @param desc 变量描述
 */
#define EK_SHELL_EXPORT_VAR_SHORT(var, variable, desc) SHELL_EXPORT_VAR_SHORT(var, variable, desc)

/**
 * @brief 导出字符型变量到 Shell
 * @param var Shell 中的变量名
 * @param variable C 代码中的变量名
 * @param desc 变量描述
 */
#define EK_SHELL_EXPORT_VAR_CHAR(var, variable, desc)  SHELL_EXPORT_VAR_CHAR(var, variable, desc)

/**
 * @brief 导出指针变量到 Shell
 * @param var Shell 中的变量名
 * @param variable C 代码中的变量名
 * @param desc 变量描述
 */
#define EK_SHELL_EXPORT_VAR_PTR(var, variable, desc)   SHELL_EXPORT_VAR_POINTER(var, variable, desc)

/**
 * @brief 导出通用变量到 Shell
 * @param var Shell 中的变量名
 * @param variable C 代码中的变量名
 * @param desc 变量描述
 */
#define EK_SHELL_EXPORT_VAR(var, variable, desc)       SHELL_EXPORT_VAL(val, value, desc)

#endif /* EK_SHELL_ENABLE */

#endif /* EK_SHELL_H */
