#ifndef __EK_STACK_H
#define __EK_STACK_H

#include "../../EK_Common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* ========================= 类型定义区 ========================= */

typedef struct
{
    void *Stack_Mem; /**< 栈内存起始地址 */
    void *Stack_TopPtr; /**< 栈顶指针 */
    size_t Stack_Capacity; /**< 栈容量 */
    bool Stack_isDynamic; /**< 是否动态分配 */
} EK_Stack_t;

/* ========================= 函数声明区 ========================= */
EK_Result_t EK_rStackCreate_Static(EK_Stack_t *stack, void *mem_ptr, size_t capacity);
EK_Stack_t *EK_pStackCreate_Dynamic(size_t capacity);
EK_Result_t EK_rStackDelete(EK_Stack_t *stack);
bool EK_bStackIsFull(EK_Stack_t *stack);
bool EK_bStackIsEmpty(EK_Stack_t *stack);
size_t EK_sStackGetRemain(EK_Stack_t *stack);
EK_Result_t EK_rStackPush(EK_Stack_t *stack, void *data, size_t data_size);
EK_Result_t EK_rStackPop(EK_Stack_t *stack, void *data_buffer, size_t data_size);

#ifdef __cplusplus
}
#endif

#endif
