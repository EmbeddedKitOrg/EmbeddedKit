/**
 * @file    EK_TasksRun.c
 * @brief   任务自动执行功能的实现文件。
 * @details 本文件实现了 EK_vTasksRun() 函数，该函数负责遍历并执行所有通过 `EK_TASK_ADD` 宏注册的任务。
 * 它依赖于链接器生成的特定符号来定位任务列表的内存区域。
 * @author  LuoShu
 * @version 1.0
 * @date    2025-10-22
 * @note    本版本目前仅在 ARM Compiler 6 (AC6) 环境下经过测试。
 */

#include "EK_TasksRun.h"

/**
 * @brief 指向 .EK_TaskEntry 段的起始地址，由链接器定义。
 * @details 这是链接器脚本生成的符号，标记了所有任务函数指针存储区域的开始位置。
 * @note    此符号为 ARMCC/AC6 链接器语法，其他工具链可能需要使用不同的符号名称和链接脚本语法。
 */
extern const EK_TaskEntry_t Image$$EK_TASK_ENTRIES$$Base;

/**
 * @brief 指向 .EK_TaskEntry 段的结束地址，由链接器定义。
 * @details 这是链接器脚本生成的符号，标记了所有任务函数指针存储区域的结束位置。
 * @note    此符号为 ARMCC/AC6 链接器语法，其他工具链可能需要使用不同的符号名称和链接脚本语法。
 */
extern const EK_TaskEntry_t Image$$EK_TASK_ENTRIES$$Limit;

/**
 * @brief   执行所有已注册的任务入口函数。
 * @details 此函数通过链接器提供的符号，获取任务函数指针数组的起止地址，然后遍历该数组并依次调用每个任务函数。
 * @note    该实现依赖于特定的链接器脚本配置（例如 ARM Compiler 的 scatter file）以正确生成任务段的起始和结束符号。
 * 目前版本仅在 ARM Compiler 6 (AC6) 工具链下测试通过，若移植到其他平台，需要修改链接脚本和对应的外部符号名称。
 */
void EK_vTasksRun(void)
{
    // 使用正确的符号名, 并进行地址和类型转换
    EK_TaskEntry_t *begin = (EK_TaskEntry_t *)&Image$$EK_TASK_ENTRIES$$Base;
    EK_TaskEntry_t *end = (EK_TaskEntry_t *)&Image$$EK_TASK_ENTRIES$$Limit;

    for (; begin < end; begin++) {
        (*begin)();
    }
}