/**
 * @file ek_conf.h
 * @brief EmbeddedKit 全局配置文件
 *
 * 此文件定义了整个 EmbeddedKit 框架的全局配置宏。
 * 所有层级（L1~L5）都可以访问此文件中的配置。
 */
#ifndef EK_CONF_H
#define EK_CONF_H

/* ========================================================================
 * RTOS配置
 * - EK_USE_RTOS: 设置为0表示不用RTOS
 * ======================================================================== */
#define EK_USE_RTOS (1)

/* ========================================================================
 * 内存管理配置 (ek_mem)
 * - EK_HEAP_NO_TLSF: 设置为1表示不使用TLSF内存池，需自定义实现具体API查看 ek_mem.h
 * - EK_HEAP_SIZE: heap的默认大小（字节）
 * ======================================================================== */
#define EK_HEAP_NO_TLSF (0)
#define EK_HEAP_SIZE    (30 * 1024)

/* ========================================================================
 * 模块功能开关
 * - EK_IO_ENABLE: 使能IO库(基于lwprintf)
 * - EK_LOG_ENABLE: 使能日志模块
 * - EK_LIST_ENABLE: 使能链表模块
 * - EK_VEC_ENABLE: 使能向量模块
 * - EK_RINGBUF_ENABLE: 使能环形缓冲区模块
 * - EK_STACK_ENABLE: 使能栈模块
 * - EK_SHELL_ENABLE: 使能shell模块   
 * ======================================================================== */
#define EK_IO_ENABLE      (1)
#define EK_LOG_ENABLE     (1)
#define EK_LIST_ENABLE    (1)
#define EK_VEC_ENABLE     (1)
#define EK_RINGBUF_ENABLE (1)
#define EK_STACK_ENABLE   (1)
#define EK_SHELL_ENABLE   (1)

/* ========================================================================
 * 日志模块配置
 * - EK_LOG_DEBUG_ENABLE: 打开调试模式
 * - EK_LOG_COLOR_ENABLE: 启用彩色日志
 * - EK_LOG_BUFFER_SIZE: 日志字符默认缓冲区大小（字节）
 * ======================================================================== */
#define EK_LOG_DEBUG_ENABLE (1)
#define EK_LOG_COLOR_ENABLE (1)
#define EK_LOG_BUFFER_SIZE  (256)

/* ========================================================================
 * 断言模块
 * - EK_ASSERT_USE_TINY: 使用最小的断言模式
 * - EK_ASSERT_WITH_LOG: 使用断言日志，确保使用断言的文件
                         都已经用 `EK_LOG_FILE_TAG` 打上标签
 * ======================================================================== */
#define EK_ASSERT_USE_TINY (1)
#define EK_ASSERT_WITH_LOG (1)

#endif // EK_CONF_H
