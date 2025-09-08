# EmbedKit 文档

欢迎来到 EmbedKit 文档中心！

## 📚 文档导航

### 快速开始
- [安装指南](getting_started/installation.md) - 如何在项目中集成 EmbedKit
- [第一个程序](getting_started/first_program.md) - 创建你的第一个 EmbedKit 应用
- [构建配置](getting_started/build_config.md) - 配置构建系统

### 核心模块
- [任务调度器](modules/scheduler.md) - 轻量级任务调度系统
- [内存管理](modules/memory.md) - 高效的内存分配策略
- [数据结构](modules/data_structures.md) - 优化的数据结构实现

### 开发资源
- [API 参考](api/README.md) - 完整的 API 文档
- [示例代码](examples/README.md) - 实际应用示例
- [最佳实践](best_practices/README.md) - 嵌入式开发建议

### 深入了解
- [架构设计](design/architecture.md) - 系统架构详解
- [移植指南](porting/overview.md) - 如何移植到新平台
- [性能优化](design/optimization.md) - 优化技巧

## 🎯 适用场景

EmbedKit 特别适合以下嵌入式应用：

- **资源受限系统** - RAM < 64KB, Flash < 256KB
- **实时应用** - 需要可预测执行时间
- **低功耗设备** - 电池供电的 IoT 设备
- **安全关键系统** - 需要静态内存分配

## 🚀 特色功能

### 1. 零动态内存分配
所有内存在编译时或初始化时分配，运行时无动态分配，避免内存碎片和不可预测的行为。

### 2. 模块化设计
按需选择模块，最小化代码体积：
```c
// embedkit_config.h
#define EK_USE_SCHEDULER  1  // 启用调度器
#define EK_USE_MEMORY     1  // 启用内存管理
#define EK_USE_QUEUE      1  // 启用队列
#define EK_USE_LIST       0  // 禁用链表
```

### 3. 硬件抽象层
简化跨平台移植：
```c
// 只需实现这些接口
void ek_port_init(void);
uint32_t ek_port_get_tick(void);
void ek_port_enter_critical(void);
void ek_port_exit_critical(void);
```

## 📊 性能数据

在 ARM Cortex-M4 @ 72MHz 上的典型性能：

| 操作 | 执行时间 | RAM 占用 |
|------|----------|----------|
| 任务切换 | < 2μs | 64 bytes/task |
| 内存分配 | < 1μs | 0 (静态池) |
| 队列操作 | < 0.5μs | 可配置 |

## 🛠️ 支持的平台

### 已验证平台
- ARM Cortex-M (M0/M0+/M3/M4/M7)
- AVR (ATmega 系列)
- MSP430
- RISC-V (RV32I)

### 编译器支持
- GCC (推荐)
- Clang/LLVM
- IAR Embedded Workbench
- Keil MDK-ARM
- TI Code Composer Studio

## 📝 版本历史

### v1.0.0 (2024-01)
- 初始发布
- 核心模块：调度器、内存池、基础数据结构

### 开发中
- 网络协议栈
- 文件系统接口
- 加密算法库

## 🤝 社区

- [GitHub 仓库](https://github.com/zuoliangyu/EmbedKit)
- [问题追踪](https://github.com/zuoliangyu/EmbedKit/issues)
- [讨论区](https://github.com/zuoliangyu/EmbedKit/discussions)

## 📄 许可证

EmbedKit 采用 MIT 许可证，可自由用于商业和非商业项目。

---

<div align="center">

**[开始使用](getting_started/installation.md)** | **[API 文档](api/README.md)** | **[示例代码](examples/README.md)**

</div>