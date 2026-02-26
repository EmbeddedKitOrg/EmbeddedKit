/**
 * @file ek_assert.h
 * @brief 断言机制
 * @author N1netyNine99
 *
 * 提供两种断言实现：
 * - ek_assert_tiny：轻量级断言，失败时进入死循环（适用于资源受限环境）
 * - ek_assert_full：完整断言，失败时调用 ek_assert_fault 输出详细信息
 *
 * @note 通过 EK_ASSERT_USE_TINY 宏选择断言实现方式
 * @warning 断言失败会导致程序停止运行
 */

#ifndef EK_ASSERT_H
#define EK_ASSERT_H

#include "../../../ek_conf.h"
#include "ek_def.h"

/**
 * @brief 是否使用轻量级断言
 * @note 1 = 使用 ek_assert_tiny（死循环），0 = 使用 ek_assert_full（输出详细信息）
 */
#ifndef EK_ASSERT_USE_TINY
#    define EK_ASSERT_USE_TINY (1)
#endif /* EK_ASSERT_USE_TINY */

/**
 * @brief 是否在断言失败时输出日志
 * @note 需要配合 EK_LOG_ENABLE 使用
 */
#ifndef EK_ASSERT_WITH_LOG
#    define EK_ASSERT_WITH_LOG (0)
#endif /* EK_ASSERT_WITH_LOG */

/**
 * @brief 轻量级断言（表达式为假时进入死循环）
 * @param expr 断言表达式
 * @note 代码体积小，适用于资源受限环境
 * @warning 断言失败会导致程序卡死在此处
 */
#define ek_assert_tiny(expr) while ((expr) == 0)

/**
 * @brief 完整断言（表达式为假时调用 ek_assert_fault）
 * @param expr 断言表达式
 * @note 会输出文件名、行号和表达式内容
 */
#define ek_assert_full(expr) (expr) ? (void)0 : ek_assert_fault(__FILE__, __LINE__, #expr)

/**
 * @brief 参数断言（根据配置选择实现方式）
 * @param expr 断言表达式
 * @note 通过 EK_ASSERT_USE_TINY 宏选择使用 tiny 或 full 版本
 */
#if EK_ASSERT_USE_TINY == 1
#    define ek_assert_param(expr) ek_assert_tiny(expr)
#else
#    define ek_assert_param(expr) ek_assert_full(expr)
#endif /* EK_ASSERT_USE_TINY */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 断言失败处理函数
 * @param file 源文件名
 * @param line 行号
 * @param expr 断言表达式字符串
 * @note 此函数会输出断言失败信息，然后进入死循环
 */
void ek_assert_fault(const char *file, uint32_t line, const char *expr);

#ifdef __cplusplus
}
#endif

#endif /* EK_ASSERT_H */
