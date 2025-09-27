/**
 * @file EK_CoroTask.h
 * @brief 协程任务管理API头文件
 * @details 定义了所有面向用户的任务操作函数，例如创建、删除、挂起、恢复、延时等。
 * @author N1ntyNine99
 * @date 2025-09-20
 * @version 1.4
 */

#ifndef __EK_COROTASK_H
#define __EK_COROTASK_H

#include "../Kernel/Kernel.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* ========================= 函数声明区 ========================= */

EK_CoroHandler_t EK_pCoroCreate(EK_CoroFunction_t task_func, void *task_arg, uint16_t priority, EK_Size_t stack_size);
EK_CoroStaticHandler_t EK_pCoroCreateStatic(EK_CoroTCB_t *static_tcb,
                                            EK_CoroFunction_t task_func,
                                            void *task_arg,
                                            uint16_t priority,
                                            void *stack,
                                            EK_Size_t stack_size);

void EK_vCoroSuspend(EK_CoroHandler_t task_handle, EK_Result_t *result);
void EK_vCoroResume(EK_CoroHandler_t task_handle, EK_Result_t *result);
void EK_vCoroDelete(EK_CoroHandler_t task_handle, EK_Result_t *result);
void EK_vCoroDelay(uint32_t xticks);
void EK_vCoroYield(void);
void EK_vCoroSetPriority(EK_CoroHandler_t task_handle, uint16_t priority, EK_Result_t *result);
EK_Size_t EK_uCoroGetStack(EK_CoroHandler_t task_handle);
EK_Size_t EK_uCoroGetStackUsage(EK_CoroHandler_t task_handle);

#ifdef __cplusplus
}
#endif

#endif
