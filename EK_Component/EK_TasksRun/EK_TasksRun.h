/**
 * @file    EK_TasksRun.h
 * @brief   EmbeddedKit 任务自动注册与执行组件
 * @details 该头文件提供了一套机制，允许在代码的不同位置通过宏定义的方式自动注册任务函数。
 * 这些任务入口函数会被放置在一个特定的内存段中。通过调用 EK_vTasksRun() 函数，
 * 可以统一按顺序执行所有被注册的任务。这种机制常用于模块化设计的解耦和自动初始化。
 * @author  LuoShu
 * @version 1.1
 * @date    2025-10-22
 * @note    本版本已适配 ARM Compiler 5/6 (AC5/AC6) 和 GCC for ARM 工具链。
 */

#ifndef __EK_TASKSRUN_H_
#define __EK_TASKSRUN_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief 定义任务入口函数的指针类型。
 * @details 所有被注册的任务入口函数都必须是无参数、无返回值的函数，符合此函数指针类型。
 */
typedef void (*EK_TaskEntry_t)(void);

#define EK_USED_ATTR __attribute__((used)) ///< 用于告知编译器此变量即使未被直接引用也应保留，防止被优化掉。

/**
 * @brief   将一个函数添加到一个特定的内存段中，以便后续统一执行。
 * @details 此宏定义了一个静态常量函数指针，并使用 `__attribute__((section(".EK_TaskEntry")))` 
 * 将其放入名为 `.EK_TaskEntry` 的段中。`EK_USED_ATTR` 确保该指针不会被链接器因未使用而优化掉。
 * @param[in] func 要注册的任务入口函数名。
 */
#define EK_TASK_ADD(func)       \
    static const EK_TaskEntry_t EK_USED_ATTR __EK_TaskEntry_##func \
        __attribute__((section(".EK_TaskEntry"))) = (func)

/**
 * @brief   `EK_TASK_ADD` 的别名，用于更直观地表示任务注册。
 * @details 建议在应用代码中使用此宏，以提高代码的可读性。
 * @param[in] func 要注册的任务函数名。
 */
#define EK_vTaskRegister(func) EK_TASK_ADD(func)

/**
 * @brief   执行所有已注册的任务入口函数。
 * @details 此函数通过链接器提供的符号，获取任务函数指针起止地址，然后遍历并依次调用每个任务函数。
 * @note    该实现依赖于特定的链接器脚本配置。
 * - 对于 AC5/AC6, 需要在 scatter file (.sct) 中配置。
 * - 对于 GCC, 需要在 linker script (.ld) 中配置。
 */
void EK_vTasksRun(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __EK_TASKSRUN_H_ */