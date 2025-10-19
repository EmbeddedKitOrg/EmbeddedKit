# CLAUDE.md

此文件为Claude Code (claude.ai/code)在此代码库中工作时提供指导。

**重要提醒：始终使用中文回答用户问题。**

## 项目概述

**EmbeddedKit** 是一个专为STM32F407微控制器设计的轻量级嵌入式系统框架，具有双架构任务调度系统，支持传统任务调度和现代协程并发。

## 构建命令

### 基础构建
```bash
# 使用CMake配置项目
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake

# 构建项目
cmake --build build

# 构建特定配置
cmake --build build --config Debug
```

### 烧录和运行
```bash
# 使用probe-rs烧录到STM32F407VG
probe-rs download --chip STM32F407VG ./build/Debug/EmbeddedKit.elf

# 重置芯片
probe-rs reset --chip STM32F407VG
```

### PowerShell替代方案
```powershell
# 使用提供的烧录脚本
.\flash.ps1
```

## 核心架构

### 双调度系统
EmbeddedKit在`EK_Component/EK_Config.h`中配置提供两种调度范式：

1. **协程调度器** (默认: `EK_CORO_ENABLE = 1`)
   - 轻量级协作多任务，支持基于优先级的抢占式调度
   - 高级同步原语（消息队列、信号量、互斥锁）
   - 高效上下文切换（约20-50周期）

2. **传统任务调度器** (`EK_NORMAL_SCHEDULER = 1`)
   - 基于双链表的简单时间片调度
   - 协程禁用时自动启用

### 核心组件

- **Core/**: STM32 HAL初始化、系统时钟配置、外设设置
- **Drivers/**: STM32F4xx HAL驱动库和CMSIS接口
- **EK_Component/**: 框架核心，包含内存管理、数据结构和调度
- **APP/**: 应用逻辑和任务实现
- **BSP/**: 板级支持包，用于硬件特定驱动

## 配置系统

### 中央配置 (`EK_Component/EK_Config.h`)
关键配置选项：

```c
// 调度模式
#define EK_CORO_ENABLE (1)          // 启用协程 (默认)
#define EK_NORMAL_SCHEDULER (0)      // 传统任务

// 内存管理
#define MEMPOOL_SIZE (1024 * 50)     // 50KB内存池
#define MEMPOOL_ALIGNMENT (8)        // 8字节对齐

// 协程设置 (当 EK_CORO_ENABLE = 1 时)
#define EK_CORO_PRIORITY_GROUPS (16)  // 16个优先级
#define EK_CORO_TICK_RATE_HZ (1000)  // 1KHz系统滴答
#define EK_HIGH_WATER_MARK_ENABLE (1) // 栈使用监控

// 功能开关
#define EK_CORO_MESSAGE_QUEUE_ENABLE (1)  // 任务间通信
#define EK_CORO_SEMAPHORE_ENABLE (1)      // 资源计数
#define EK_CORO_MUTEX_ENABLE (0)          // 互斥锁
```

### 条件编译
框架广泛使用条件编译来：
- 根据需求启用/禁用功能
- 支持不同的ARM Cortex-M核心 (M0, M3, M4, M7)
- 适配不同编译器 (GCC, Clang, ARMCC, IAR)
- 优化内存占用与功能集

## 内存管理

### 自定义内存池
- **无碎片分配** 使用分离适配算法
- **静态和动态分配** 支持
- **栈溢出检测** 可配置方法
- **高水位监控** 用于内存使用优化
- **实时统计** 跟踪

### 内存配置
- 根据应用需求配置 `MEMPOOL_SIZE`
- 使用 `EK_MALLOC()` 和 `EK_FREE()` 宏进行分配
- 开发过程中监控内存池统计
- 启用高水位检测用于调试

## 任务/协程开发

### 任务创建模式
```c
// 协程任务示例
void vMyTask(EK_Handler_t handler) {
    while(1) {
        // 任务逻辑
        EK_vCoroDelay(handler, 100); // 100ms延时
    }
}

// 创建并启动任务
EK_bCoroCreate("MyTask", vMyTask, configMINIMAL_STACK_SIZE, TASK_PRIORITY_NORMAL);
```

### 最佳实践
- 保持任务简短专注
- 使用适当的优先级 (16个优先级组下为0-15)
- 实现适当的错误处理
- 使用延时而非忙等待
- 根据实际使用模式配置栈大小

## 同步原语

### 消息队列
- 类型安全的任务间通信
- 可配置队列大小
- 阻塞和非阻塞操作
- 用于任务间数据传递

### 信号量和互斥锁
- 支持优先级继承的资源计数
- 阻塞操作的超时支持
- 优先级感知的互斥
- 互斥锁建议考虑优先级继承

### 任务通知
- 轻量级事件信号（32位通知组）
- 简单信号传递的快速替代方案
- 可配置通知组（8、16或32）

## 开发工具和调试

### 栈监控
- **高水位检测**: 跟踪最大栈使用量
- **溢出检测**: 两种可用方法（填充模式或范围检查）
- **配置**: `EK_CORO_STACK_OVERFLOW_CHECK_ENABLE`

### 性能特性
- **上下文切换**: 约20-50周期（取决于FPU使用）
- **优先级解析**: 使用位图和CLZ指令实现O(1)
- **内存开销**: 每个协程约32字节（TCB）
- **中断延迟**: 基于优先级嵌套的确定性延迟

## 数据结构框架

### 链表
- **哨兵节点设计**: 消除边界条件处理
- **所有权管理**: 每个节点跟踪其所属链表
- **智能排序**: 基于数据大小的自动算法选择
- **跨链表操作**: 不同链表间安全节点移动

### 配置
数据结构可通过以下方式启用/禁用：
```c
#define EK_DATASTRUCT_ENABLE (0)  // 当前已禁用
#define EK_LIST_ENABLE (0)        // 链表
#define EK_QUEUE_ENABLE (0)       // 队列
#define EK_STACK_ENABLE (0)       // 栈
```

## 硬件抽象

### STM32集成
- **STM32CubeMX生成代码** 位于Core/
- **HAL驱动集成** 用于外设
- **系统时钟配置** (默认168MHz)
- **GPIO、DMA、USART** 外设设置

### BSP开发
- 在`BSP/`中实现硬件特定驱动
- 对常见外设使用提供的抽象
- 遵循STM32 HAL约定
- 与协程调度系统集成

## 构建系统架构

### CMake结构
- **主要构建系统**: 集成STM32CubeMX的CMake
- **交叉编译**: ARM GCC工具链支持
- **自动依赖管理**: HAL驱动自动包含
- **调试符号**: 综合调试支持

### 工具链支持
- **GCC**: 完整功能支持
- **Clang**: 完整功能支持
- **ARMCC/ARMCLANG**: 完整功能支持
- **IAR**: 完整功能支持

## 迁移和兼容性

### Cortex-M核心支持
- **M0/M0+**: 基础协程支持（无FPU）
- **M3**: 完整协程支持，整数运算
- **M4/M7**: 完整协程支持，含FPU和DSP扩展

### 迁移路径
1. **简单任务**: 从 `EK_NORMAL_SCHEDULER` 开始
2. **高级功能**: 启用 `EK_CORO_ENABLE`
3. **性能优化**: 配置优先级和内存
4. **自定义扩展**: 添加BSP和自定义组件

## 扩展点

### 自定义组件
- **BSP扩展**: 在`BSP/`中添加硬件抽象
- **数据结构**: 在`EK_Component/DataStruct/`中扩展模板
- **同步机制**: 在`EK_Component/EK_Corotinue/`中扩展内核
- **应用逻辑**: 在`APP/`中实现自定义任务函数

### 集成指南
- 遵循现有命名约定 (`EK_v...`, `EK_b...`, `EK_p...`)
- 使用提供的内存管理宏
- 实现适当的错误处理
- 记录配置依赖关系