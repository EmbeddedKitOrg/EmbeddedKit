/**
 * @file EK_Stack.h
 * @brief 栈数据结构头文件
 * @details 定义了栈的数据结构和操作接口
 * @author N1ntyNine99
 * @date 2025-09-08
 * @version 1.0
 */

#ifndef __EK_STACK_H
#define __EK_STACK_H

#include "../../EK_Config.h"

/* 数据结构模块条件编译 */
#if (EK_DATASTRUCT_ENABLE == 1)
#if (EK_STACK_ENABLE == 1)

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* ========================= 类型定义区 ========================= */

typedef struct
{
    void *Stack_Mem; /**< 栈内存起始地址 */
    void *Stack_TopPtr; /**< 栈顶指针 */
    EK_Size_t Stack_Capacity; /**< 栈容量 */
    bool Stack_isDynamic; /**< 是否动态分配 */
} EK_Stack_t;

/* ========================= 函数声明区 ========================= */
EK_Result_t EK_rStackCreateStatic(EK_Stack_t *stack, void *mem_ptr, EK_Size_t capacity);
EK_Stack_t *EK_pStackCreate(EK_Size_t capacity);
EK_Result_t EK_rStackDelete(EK_Stack_t *stack);
bool EK_bStackIsFull(EK_Stack_t *stack);
bool EK_bStackIsEmpty(EK_Stack_t *stack);
EK_Size_t EK_uStackGetRemain(EK_Stack_t *stack);
EK_Result_t EK_rStackPush(EK_Stack_t *stack, void *data, EK_Size_t data_size);
EK_Result_t EK_rStackPop(EK_Stack_t *stack, void *data_buffer, EK_Size_t data_size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* EK_STACK_ENABLE */
#endif /* EK_DATASTRUCT_ENABLE */

#endif /* __EK_STACK_H */
