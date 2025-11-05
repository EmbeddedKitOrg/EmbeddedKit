/**
 * @file           lwprintf_EK_Coro.c
 * @brief          lwprintf的EK协程适配层实现文件
 * @details        使用EK_CoroSemaphore互斥锁为lwprintf提供线程安全的协程环境支持
 * @author         N1ntyNine99
 * @date           2025-10-24
 * @version        v1.0
 */

/* ========================= 头文件包含区 ========================= */
#include "Inc/lwprintf_sys.h"
#include "../../EK_Corotinue/Inc/EK_CoroSemaphore.h"

/* ========================= 条件编译 ========================= */
#if (EK_LWPRINTF_ENABLE == 1) && LWPRINTF_CFG_OS && !__DOXYGEN__

/* ========================= 内部数据结构 ========================= */

/**
 * @brief          lwprintf协程互斥锁结构体
 * @details        封装EK_CoroSemaphore的互斥锁，提供lwprintf所需的互斥锁功能
 */
static struct
{
    EK_CoroSem_t *mutex_handle; /**< EK协程互斥锁句柄 */
    bool is_initialized; /**< 初始化标志 */
} lwprintf_ek_mutex = {NULL, false};

/* ========================= 内部函数声明 ========================= */

/**
 * @brief          初始化lwprintf协程互斥锁
 * @return         成功返回1，失败返回0
 */
static uint8_t u8_lwprintf_ek_mutex_init(void);

/* ========================= 公开API实现 ========================= */

/**
 * @brief          创建新的互斥锁并分配句柄
 * @param[out]     m: 输出变量，用于保存互斥锁句柄
 * @return         成功返回1，失败返回0
 * @note           使用EK_CoroSemaphore的互斥锁功能实现
 */
uint8_t lwprintf_sys_mutex_create(LWPRINTF_CFG_OS_MUTEX_HANDLE *m)
{
    // 参数有效性检查
    if (m == NULL)
    {
        return 0;
    }

    // 如果尚未初始化全局互斥锁，先初始化
    if (!lwprintf_ek_mutex.is_initialized)
    {
        if (!u8_lwprintf_ek_mutex_init())
        {
            return 0;
        }
    }

    // 分配新的互斥锁
    *m = EK_pSemGenericCreate(1, 1, true, false); // 创建互斥锁
    return (*m != NULL) ? 1 : 0;
}

/**
 * @brief          检查互斥锁句柄是否有效
 * @param[in]      m: 要检查的互斥锁句柄
 * @return         有效返回1，无效返回0
 */
uint8_t lwprintf_sys_mutex_isvalid(LWPRINTF_CFG_OS_MUTEX_HANDLE *m)
{
    // 参数有效性检查
    if (m == NULL || *m == NULL)
    {
        return 0;
    }

    // 检查互斥锁是否为有效的EK协程互斥锁
    EK_CoroSem_t *sem = (EK_CoroSem_t *)(*m);
    return (sem->Sem_isMutex == true) ? 1 : 0;
}

/**
 * @brief          等待互斥锁直到可用（无限等待时间）
 * @param[in]      m: 要等待的互斥锁句柄
 * @return         成功返回1，失败返回0
 * @note           使用EK_MAX_DELAY进行无限等待
 */
uint8_t lwprintf_sys_mutex_wait(LWPRINTF_CFG_OS_MUTEX_HANDLE *m)
{
    // 参数有效性检查
    if (m == NULL || *m == NULL)
    {
        return 0;
    }

    // 使用EK协程信号量的获取功能
    EK_Result_t result = EK_rSemTake((EK_CoroSemHanlder_t)(*m), EK_MAX_DELAY);
    return (result == EK_OK) ? 1 : 0;
}

/**
 * @brief          释放已锁定的互斥锁
 * @param[in]      m: 要释放的互斥锁句柄
 * @return         成功返回1，失败返回0
 */
uint8_t lwprintf_sys_mutex_release(LWPRINTF_CFG_OS_MUTEX_HANDLE *m)
{
    // 参数有效性检查
    if (m == NULL || *m == NULL)
    {
        return 0;
    }

    // 使用EK协程信号量的释放功能
    EK_Result_t result = EK_rSemGive((EK_CoroSemHanlder_t)(*m));
    return (result == EK_OK) ? 1 : 0;
}

/* ========================= 内部函数实现 ========================= */

/**
 * @brief          初始化lwprintf协程互斥锁
 * @return         成功返回1，失败返回0
 * @note           这是一个内部函数，用于初始化全局互斥锁状态
 */
static uint8_t u8_lwprintf_ek_mutex_init(void)
{
    // 创建全局互斥锁
    lwprintf_ek_mutex.mutex_handle = EK_pSemGenericCreate(1, 1, true, false);

    if (lwprintf_ek_mutex.mutex_handle == NULL)
    {
        lwprintf_ek_mutex.is_initialized = false;
        return 0;
    }

    lwprintf_ek_mutex.is_initialized = true;
    return 1;
}

#endif /* (EK_LWPRINTF_ENABLE == 1) && LWPRINTF_CFG_OS && !__DOXYGEN__ */