/**
 * @file EK_Stack.c
 * @brief 栈数据结构实现文件
 * @details 实现了基于连续内存的LIFO栈数据结构，支持动态和静态内存管理。
 *          提供栈的创建、销毁、入栈、出栈等基本操作，
 *          以及栈状态检查和空间管理功能。
 * @note 栈采用字节流方式存储数据，支持任意类型数据的存储
 * @warning 使用前请确保正确初始化栈结构
 * 
 * @author N1ntyNine99
 * @date 2025-09-12
 * @version 1.0
 */

#include "EK_Stack.h"

/* ========================= 内部函数定义区 ========================= */

/**
 * @brief 获取栈的顶端边界地址
 * @details 计算栈内存的最大可用地址，用于边界检查
 * @param stack 栈指针
 * @return void* 栈顶端边界地址，失败返回NULL
 * @note 返回的地址是栈满时栈顶指针应该达到的位置
 */
static inline void *v_stack_get_top(EK_Stack_t *stack)
{
    if (stack == NULL) return NULL;

    return (uint8_t *)stack->Stack_Mem + stack->Stack_Capacity;
}

/* ========================= 公用API函数定义区 ========================= */

/**
 * @brief 静态创建栈（使用用户提供的内存）
 * @details 在已分配的栈内存上初始化栈结构
 * @param stack 指向已分配内存的栈结构体指针
 * @param mem_ptr 用户提供的栈空间内存指针
 * @param capacity 栈容量（字节数）
 * @return EK_Result_t 操作结果
 * @note 适用于静态分配场景，栈结构体和栈空间内存都由用户管理
 */

EK_Result_t EK_rStackCreate_Static(EK_Stack_t *stack, void *mem_ptr, EK_Size_t capacity)
{
    if (mem_ptr == NULL || stack == NULL) return EK_NULL_POINTER;
    if (capacity == 0) return EK_INVALID_PARAM;

    stack->Stack_Mem = mem_ptr;
    stack->Stack_Capacity = capacity;
    stack->Stack_TopPtr = mem_ptr; // 栈顶指针初始化为栈底
    stack->Stack_isDynamic = false;

    return EK_OK;
}

/**
 * @brief 动态创建栈（使用动态内存分配）
 * @details 动态分配内存并初始化栈结构
 * @param capacity 栈容量（字节数）
 * @return EK_Stack_t* 创建的栈指针
 * @note 适用于动态分配场景，栈内存由malloc管理，需要使用EK_rStackDelete释放
 */
EK_Stack_t *EK_pStackCreate_Dynamic(EK_Size_t capacity)
{
    if (capacity == 0) return NULL;

    // 为栈结构体分配内存
    EK_Stack_t *stack = (EK_Stack_t *)EK_MALLOC(sizeof(EK_Stack_t));
    if (stack == NULL)
    {
        return NULL;
    }

    // 为栈空间分配内存
    void *buffer = EK_MALLOC(capacity);
    if (buffer == NULL)
    {
        EK_FREE(stack);
        return NULL;
    }

    stack->Stack_Mem = buffer;
    stack->Stack_Capacity = capacity;
    stack->Stack_TopPtr = buffer; // 栈顶指针初始化为栈底
    stack->Stack_isDynamic = true;

    return stack;
}

/**
 * @brief 删除栈并释放相关资源
 * @details 对于动态栈会释放所有分配的内存，对于静态栈只清空内容
 * @param stack 要删除的栈指针
 * @return EK_Result_t 删除结果
 * @note 对于动态栈会释放栈空间和栈结构内存，对于静态栈只清空内容并重置状态
 * @warning 删除后动态栈指针将变为无效，不应再使用
 */
EK_Result_t EK_rStackDelete(EK_Stack_t *stack)
{
    if (stack == NULL) return EK_NULL_POINTER;

    // 如果是动态分配的内存，释放栈空间和栈结构
    if (stack->Stack_isDynamic == true)
    {
        if (stack->Stack_Mem != NULL)
        {
            EK_FREE(stack->Stack_Mem);
        }
        EK_FREE(stack);
        return EK_OK;
    }
    // 静态栈只清空内容和重置指针
    else
    {
        if (stack->Stack_Mem != NULL)
        {
            EK_vMemSet(stack->Stack_Mem, 0, stack->Stack_Capacity);
        }
        stack->Stack_TopPtr = stack->Stack_Mem; // 重置为栈底
        return EK_OK;
    }
}

/**
 * @brief 检查栈是否已满
 * @details 通过比较栈顶指针与栈边界来判断栈是否已满
 * @param stack 要检查的栈指针
 * @return bool 栈满返回true，未满或栈指针无效返回false
 * @note 当栈指针为NULL时返回false，表示检查失败
 */
bool EK_bStackIsFull(EK_Stack_t *stack)
{
    if (stack == NULL) return false;

    void *stack_top = v_stack_get_top(stack);
    if (stack_top == NULL) return false;

    return (stack->Stack_TopPtr >= stack_top);
}

/**
 * @brief 检查栈是否为空
 * @details 通过比较栈顶指针与栈底地址来判断栈是否为空
 * @param stack 要检查的栈指针
 * @return bool 栈为空返回true，非空或栈指针无效返回false
 * @note 当栈指针为NULL时返回false，表示检查失败
 */
bool EK_bStackIsEmpty(EK_Stack_t *stack)
{
    if (stack == NULL) return false;

    return (stack->Stack_TopPtr == stack->Stack_Mem);
}

/**
 * @brief 获取栈剩余可用空间大小
 * @details 计算栈中还可以存储多少字节的数据
 * @param stack 栈指针
 * @return EK_Size_t 返回栈剩余可用的字节数，栈指针无效时返回0
 * @note 返回值表示还可以向栈中压入多少字节的数据
 */
EK_Size_t EK_sStackGetRemain(EK_Stack_t *stack)
{
    if (stack == NULL) return 0;

    void *stack_top = v_stack_get_top(stack);
    if (stack_top == NULL) return 0;

    int remain = (uint8_t *)stack_top - (uint8_t *)stack->Stack_TopPtr;
    return (remain > 0 ? (EK_Size_t)remain : 0);
}

/**
 * @brief 向栈中压入数据（入栈操作）
 * @details 将指定数据压入栈顶，栈顶指针向上移动
 * @param stack 栈指针
 * @param data 要压入的数据指针
 * @param data_size 数据大小（字节数）
 * @return EK_Result_t 入栈结果
 * @note 会检查栈剩余空间是否足够，数据会被复制到栈内部空间
 * @warning 确保data指向的内存区域至少有data_size字节有效数据
 */
EK_Result_t EK_rStackPush(EK_Stack_t *stack, void *data, EK_Size_t data_size)
{
    if (data == NULL || stack == NULL) return EK_NULL_POINTER;
    if (data_size == 0) return EK_INVALID_PARAM;

    // 检测剩余空间是否足够
    if (EK_sStackGetRemain(stack) < data_size) return EK_INSUFFICIENT_SPACE;

    // 写入数据
    EK_vMemCpy(stack->Stack_TopPtr, data, data_size);

    // 移动栈顶指针
    stack->Stack_TopPtr = (uint8_t *)stack->Stack_TopPtr + data_size;

    return EK_OK;
}

/**
 * @brief 从栈中弹出数据（出栈操作）
 * @details 从栈顶取出指定大小的数据，栈顶指针向下移动
 * @param stack 栈指针
 * @param data_buffer 用于接收出栈数据的缓冲区指针
 * @param data_size 要弹出的数据大小（字节数）
 * @return EK_Result_t 出栈结果
 * @note 会检查栈中数据是否足够，数据会从栈中移除并复制到data_buffer中
 * @warning 确保data_buffer有足够空间存储data_size字节的数据
 */
EK_Result_t EK_rStackPop(EK_Stack_t *stack, void *data_buffer, EK_Size_t data_size)
{
    if (data_buffer == NULL || stack == NULL) return EK_NULL_POINTER;
    if (data_size == 0) return EK_INVALID_PARAM;

    // 检测是否为空
    if (EK_bStackIsEmpty(stack)) return EK_EMPTY;

    // 检测栈中是否有足够的数据
    EK_Size_t current_size = (uint8_t *)stack->Stack_TopPtr - (uint8_t *)stack->Stack_Mem;
    if (current_size < data_size) return EK_INSUFFICIENT_SPACE;

    // 移动栈顶指针
    stack->Stack_TopPtr = (uint8_t *)stack->Stack_TopPtr - data_size;

    // 读取数据
    EK_vMemCpy(data_buffer, stack->Stack_TopPtr, data_size);

    return EK_OK;
}