/**
 * @file    EK_TasksRun.c
 * @brief   任务自动执行功能的实现文件。
 * @details 本文件实现了 EK_vTasksRun() 函数，该函数负责遍历并执行所有通过 `EK_TASK_ADD` 宏注册的任务。
 * 它依赖于链接器生成的特定符号来定位任务列表的内存区域。
 * @author  LuoShu
 * @version 1.1
 * @date    2025-10-22
 * @note    本版本已适配 ARM Compiler 5/6 (AC5/AC6) 和 GCC for ARM 工具链。
 */

#include "EK_TasksRun.h"

/*
 * 根据不同的工具链定义不同的段起始和结束符号。
 * - AC5/AC6 (ARM Linker) 使用 Image$$...$$Base/Limit 格式。
 * - GCC (GNU Linker) 使用 __..._start/end 格式，需要在链接脚本中定义。
 */
#if defined(__CC_ARM) || defined(__ARMCC_VERSION) /* AC5 and AC6 */
    /**
     * @brief 指向 .EK_TaskEntry 段的起始地址，由 ARM 链接器在分散加载文件中定义。
     * @note  此符号为 ARMCC/AC6 链接器语法。需要在 .sct 文件中定义 EK_TASK_ENTRIES 执行域。
     */
    extern const EK_TaskEntry_t Image$$EK_TASK_ENTRIES$$Base;
    /**
     * @brief 指向 .EK_TaskEntry 段的结束地址，由 ARM 链接器在分散加载文件中定义。
     */
    extern const EK_TaskEntry_t Image$$EK_TASK_ENTRIES$$Limit;

#elif defined(__GNUC__) /* GCC for ARM */
    /**
     * @brief 指向 .EK_TaskEntry 段的起始地址，由 GCC 链接器在链接脚本中定义。
     * @note  此符号为 GCC 链接器语法。需要在 .ld 文件中定义。
     */
    extern const EK_TaskEntry_t __EK_TaskEntry_start[];
    /**
     * @brief 指向 .EK_TaskEntry 段的结束地址，由 GCC 链接器在链接脚本中定义。
     */
    extern const EK_TaskEntry_t __EK_TaskEntry_end[];

#else
    #error "Unsupported toolchain. Please adapt EK_TasksRun.c for your compiler."
#endif

/**
 * @brief   执行所有已注册的任务入口函数。
 * @details 此函数通过链接器提供的符号，获取任务函数指针数组的起止地址，然后遍历该数组并依次调用每个任务函数。
 * @note    该实现依赖于特定的链接器脚本配置。
 * - 对于 AC5/AC6, 需要在 scatter file (.sct) 中配置。
 * - 对于 GCC, 需要在 linker script (.ld) 中配置。
 */
void EK_vTasksRun(void)
{
#if defined(__CC_ARM) || defined(__ARMCC_VERSION) /* AC5 and AC6 */
    const EK_TaskEntry_t *pFunc = &Image$$EK_TASK_ENTRIES$$Base;
    const EK_TaskEntry_t *pEnd  = &Image$$EK_TASK_ENTRIES$$Limit;
#elif defined(__GNUC__) /* GCC for ARM */
    const EK_TaskEntry_t *pFunc = __EK_TaskEntry_start;
    const EK_TaskEntry_t *pEnd  = __EK_TaskEntry_end;
#endif
    for (; pFunc < pEnd; pFunc++) {
        (*pFunc)();
    }
}