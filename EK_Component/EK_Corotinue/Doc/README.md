# EmbeddedKit 协程系统文档

## 📖 文档目录

欢迎使用 EmbeddedKit 协程系统！这里提供了完整的文档帮助您快速上手和深入使用。

### 🚀 快速开始

| 文档 | 描述 | 适合读者 | 阅读时间 |
|------|------|----------|----------|
| [📋 快速入门指南](快速入门指南.md) | 5分钟快速上手协程系统 | 初学者 | 5-10分钟 |
| [🔧 API速查表](#api速查表) | 常用API快速参考 | 所有开发者 | 随时查阅 |

---

### 📚 深入学习

| 文档 | 描述 | 适合读者 | 阅读时间 |
|------|------|----------|----------|
| [🛠️ 移植指南](移植指南.md) | 详细的移植步骤和配置说明 | 系统集成工程师 | 30-60分钟 |
| [📖 API参考手册](API参考手册.md) | 完整的API函数详细说明 | 应用开发工程师 | 60-120分钟 |

---

## 🎯 推荐学习路径

### 初学者路径
1. **[📋 快速入门指南](快速入门指南.md)** - 了解基本概念
2. **实践** - 创建您的第一个多任务应用
3. **[🔧 API速查表](#api速查表)** - 掌握常用API

### 进阶用户路径
1. **[🛠️ 移植指南](移植指南.md)** - 深入理解系统配置
2. **[📖 API参考手册](API参考手册.md)** - 掌握所有API细节
3. **实践** - 构建复杂的应用程序

### 系统集成工程师
1. **[🛠️ 移植指南](移植指南.md)** - 完整移植流程
2. **[📋 快速入门指南](快速入门指南.md)** - 快速验证
3. **[📖 API参考手册](API参考手册.md)** - API参考

---

## 🔧 API速查表

### 任务管理
```c
// 创建任务
EK_CoroHandler_t task = EK_pCoroCreate(TaskFunc, arg, priority, stack_size);

// 删除任务
EK_vCoroDelete(task, &result);

// 延时操作
EK_vCoroDelay(ticks);
EK_vCoroDelayUntil(wake_time);

// 任务控制
EK_vCoroSuspend(task, &result);
EK_vCoroResume(task, &result);
EK_vCoroYield();

// 任务信息
EK_Size_t stack_size = EK_uCoroGetStack(task);
EK_Size_t water_mark = EK_uCoroGetHighWaterMark(task);
```

### 消息队列
```c
// 创建队列
EK_CoroMsgHanler_t queue = EK_pMsgCreate(item_size, item_amount);

// 消息操作
EK_rMsgSend(queue, &data, timeout);
EK_rMsgReceive(queue, &buffer, timeout);
EK_rMsgDelete(queue);
```

### 内存管理
```c
// 协程专用内存管理
void *ptr = EK_CORO_MALLOC(size);
EK_CORO_FREE(ptr);
```

### 内核控制
```c
// 系统初始化
EK_vKernelInit();
EK_vKernelStart();

// 中断处理
void SysTick_Handler(void) { EK_vTickHandler(); }
void PendSV_Handler(void) { EK_vPendSVHandler(); }
```

---

## ⚙️ 配置参数速查

### 系统配置 (EK_Config.h)
```c
#define EK_CORO_ENABLE (1)                    // 启用协程系统
#define MEMPOOL_SIZE (10240)                  // 内存池大小
#define EK_CORO_PRIORITY_GROUPS (16)           // 优先级组数量
#define EK_CORO_TICK_RATE_HZ (1000)           // 系统时钟频率
#define EK_CORO_MESSAGE_QUEUE_ENABLE (1)         // 消息队列支持
```

### 协程配置 (EK_CoroConfig.h)
```c
#include "stm32f4xx_hal.h"                   // MCU头文件
#define EK_CORO_SYSTEM_FREQ (SystemCoreClock) // 系统时钟
#define EK_CORO_STACK_OVERFLOW_CHECK_ENABLE (1)      // 栈溢出检测
#define EK_CORO_IDLE_TASK_STACK_SIZE (256)    // 空闲任务栈大小
```

---

## 🐛 常见问题解答

### Q: 程序启动后崩溃怎么办？
**A**: 检查：
1. 内存池初始化：`EK_bMemPool_Init()`
2. 中断配置：SysTick 和 PendSV
3. 栈大小：建议至少256字节

### Q: 任务不切换怎么办？
**A**: 检查：
1. 是否调用了 `EK_vKernelStart()`
2. PendSV中断是否正常触发
3. 是否所有任务都在阻塞状态

### Q: 内存分配失败怎么办？
**A**: 解决方案：
1. 增加 `MEMPOOL_SIZE`
2. 检查内存泄漏
3. 使用静态分配

### Q: 如何调试栈溢出？
**A**: 启用检测：
```c
#define EK_CORO_STACK_OVERFLOW_CHECK_ENABLE (1)
EK_Size_t water_mark = EK_uCoroGetHighWaterMark(task);
```

---

## 📖 文档详细内容

### 📋 快速入门指南
- **目标**: 5分钟快速上手
- **内容**:
  - 环境准备
  - 5分钟快速上手步骤
  - 完整示例代码
  - 常用API速查
  - 调试技巧
- **适合**: 初学者

### 🛠️ 移植指南
- **目标**: 完整的移植流程
- **内容**:
  - 系统架构介绍
  - 详细的移植步骤
  - 配置参数详解
  - 中断处理配置
  - 最佳实践
  - 常见问题解决
  - 完整示例
- **适合**: 系统集成工程师

### 📖 API参考手册
- **目标**: 完整的API文档
- **内容**:
  - 所有API函数详解
  - 参数说明
  - 返回值说明
  - 使用示例
  - 数据类型定义
  - 错误代码说明
- **适合**: 应用开发工程师

---

## 🔄 版本信息

- **当前版本**: v1.0
- **更新日期**: 2025-10-07
- **兼容性**: ARM Cortex-M0/M3/M4/M7
- **编译器支持**: GCC, ARMCC, IAR, Clang

---

## 📞 技术支持

如果您在使用过程中遇到问题：

1. **查阅文档** - 首先查看相关文档的常见问题部分
2. **检查配置** - 确认配置参数设置正确
3. **查看示例** - 参考文档中的示例代码
4. **提交问题** - 提供详细的问题描述和环境信息

---

## 📝 文档更新记录

### v1.0 (2025-10-07)
- ✅ 创建完整文档体系
- ✅ 快速入门指南
- ✅ 详细移植指南
- ✅ 完整API参考手册
- ✅ 常见问题解答

---

## 🎉 开始使用

现在您可以开始使用 EmbeddedKit 协程系统了！

**建议学习路径**:
1. 阅读 [📋 快速入门指南](快速入门指南.md)
2. 按照指南完成第一个示例
3. 阅读 [🛠️ 移植指南](移植指南.md) 了解高级特性
4. 参考 [📖 API参考手册](API参考手册.md) 开发您的应用

祝您使用愉快！🚀