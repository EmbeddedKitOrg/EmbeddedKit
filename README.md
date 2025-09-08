# EmbedKit

<div align="center">

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C Standard](https://img.shields.io/badge/C-99/11-green.svg)](https://en.wikipedia.org/wiki/C99)
[![Platform](https://img.shields.io/badge/platform-ARM%20|%20AVR%20|%20MSP430%20|%20RISC--V-lightgrey)](https://github.com/zuoliangyu/EmbedKit)
[![Documentation](https://img.shields.io/badge/docs-docsify-brightgreen.svg)](https://github.com/zuoliangyu/EmbedKit)

**轻量级、模块化的嵌入式C语言开发库**

[快速开始](#快速开始) • [文档](docs/) • [示例](examples/) • [贡献指南](CONTRIBUTING.md)

</div>

---

## 📚 简介

EmbedKit 是一个专为资源受限的嵌入式系统设计的轻量级C语言库集合。它提供了经过优化的常用数据结构、内存管理工具和实时任务调度器，帮助开发者快速构建可靠的嵌入式应用。

### ✨ 核心特性

- 🚀 **极致轻量** - 最小ROM占用，可配置的RAM使用
- 🔧 **模块化设计** - 按需引入，零依赖架构
- ⚡ **高性能** - 针对嵌入式处理器优化的算法实现
- 🛡️ **安全可靠** - 静态内存分配，可预测的执行时间
- 📖 **文档完善** - 详细的API文档和使用示例
- 🎯 **标准兼容** - 支持C99/C11标准，跨平台移植性强

## 🏗️ 模块架构

```
EmbedKit/
├── core/               # 核心基础模块
│   ├── common/        # 通用定义和宏
│   └── config/        # 配置管理
├── scheduler/         # 任务调度器
│   ├── task.h        # 任务管理
│   └── timer.h       # 定时器服务
├── memory/           # 内存管理
│   ├── pool.h        # 内存池
│   └── heap.h        # 动态内存管理
├── data_structures/  # 数据结构
│   ├── list.h        # 链表
│   ├── queue.h       # 队列
│   ├── stack.h       # 栈
│   └── ring_buffer.h # 环形缓冲区
├── utils/            # 实用工具
│   ├── crc.h         # CRC校验
│   ├── debug.h       # 调试工具
│   └── log.h         # 日志系统
└── drivers/          # 硬件抽象层
    ├── gpio.h        # GPIO接口
    ├── uart.h        # 串口通信
    └── spi.h         # SPI接口
```

## 🚀 快速开始

### 环境要求

- **编译器**: GCC 4.8+ / Clang 3.4+ / IAR / Keil MDK
- **标准**: C99 或更高
- **RAM**: 最低 2KB（基础功能）
- **ROM**: 最低 8KB（包含核心模块）

### 安装

#### 方法1：作为Git子模块

```bash
git submodule add https://github.com/zuoliangyu/EmbedKit.git libs/EmbedKit
git submodule update --init --recursive
```

#### 方法2：直接下载

```bash
wget https://github.com/zuoliangyu/EmbedKit/releases/latest/download/embedkit.zip
unzip embedkit.zip -d libs/
```

### 基础示例

```c
#include "embedkit/scheduler/task.h"
#include "embedkit/memory/pool.h"
#include "embedkit/data_structures/queue.h"

// 内存池配置
#define POOL_SIZE 1024
static uint8_t memory_pool[POOL_SIZE];

// 任务函数
void led_task(void* param) {
    // 任务逻辑
    toggle_led();
}

int main(void) {
    // 初始化内存池
    ek_pool_t* pool = ek_pool_init(memory_pool, POOL_SIZE, 32);
    
    // 初始化任务调度器
    ek_scheduler_init();
    
    // 创建任务
    ek_task_create(led_task, NULL, 100, 1);  // 100ms周期，优先级1
    
    // 启动调度器
    ek_scheduler_start();
    
    // 不会到达这里
    return 0;
}
```

## 📦 核心模块

### 任务调度器 (Scheduler)

轻量级的协作式任务调度器，支持：
- 优先级调度
- 周期性任务
- 软定时器
- 低功耗支持

[详细文档 →](docs/modules/scheduler.md)

### 内存管理 (Memory)

高效的内存管理方案：
- **内存池**: 固定大小块分配，O(1)时间复杂度
- **堆管理**: 可选的动态内存分配
- **内存对齐**: 自动处理平台对齐要求

[详细文档 →](docs/modules/memory.md)

### 数据结构 (Data Structures)

优化的常用数据结构：
- **链表**: 单向/双向链表
- **队列**: FIFO队列，支持优先级
- **环形缓冲**: 无锁设计，适合中断环境
- **栈**: 固定大小栈实现

[详细文档 →](docs/modules/data_structures.md)

## 🎯 使用场景

EmbedKit 适用于以下嵌入式应用场景：

- 🏭 **工业控制** - PLC、传感器节点、执行器控制
- 🏠 **智能家居** - IoT设备、智能开关、环境监测
- 🚗 **汽车电子** - ECU、车身控制、仪表系统
- 🔬 **医疗设备** - 监护仪、便携式诊断设备
- 🤖 **机器人** - 电机控制、传感器融合、路径规划

## 📊 性能指标

| 操作 | 时间复杂度 | 典型执行时间 (Cortex-M4 @ 72MHz) |
|------|------------|-----------------------------------|
| 内存池分配 | O(1) | < 100 cycles |
| 任务切换 | O(1) | < 200 cycles |
| 队列入队 | O(1) | < 50 cycles |
| 链表插入 | O(1) | < 30 cycles |

## 🔧 配置

EmbedKit 支持通过配置文件定制功能：

```c
// embedkit_config.h
#define EK_USE_SCHEDULER    1  // 启用任务调度器
#define EK_USE_MEMORY_POOL  1  // 启用内存池
#define EK_MAX_TASKS        16 // 最大任务数
#define EK_TICK_RATE_HZ     1000 // 系统tick频率
```

## 📚 文档

- [API参考手册](docs/api/README.md)
- [设计文档](docs/design/README.md)
- [移植指南](docs/porting/README.md)
- [最佳实践](docs/best_practices.md)
- [常见问题](docs/faq.md)

## 🧪 测试

运行单元测试：

```bash
make test
```

运行性能测试：

```bash
make benchmark
```

## 🤝 贡献

我们欢迎所有形式的贡献！请查看[贡献指南](CONTRIBUTING.md)了解如何参与项目。

### 开发流程

1. Fork 项目
2. 创建特性分支 (`git checkout -b feature/amazing-feature`)
3. 提交更改 (`git commit -m 'Add amazing feature'`)
4. 推送到分支 (`git push origin feature/amazing-feature`)
5. 创建 Pull Request

## 📄 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

## 👥 团队

- **维护者**: [zuoliangyu](https://github.com/zuoliangyu)
- **贡献者**: 查看[贡献者列表](https://github.com/zuoliangyu/EmbedKit/contributors)

## 🙏 致谢

感谢所有为 EmbedKit 做出贡献的开发者！

## 📮 联系我们

- **Issues**: [GitHub Issues](https://github.com/zuoliangyu/EmbedKit/issues)
- **讨论**: [GitHub Discussions](https://github.com/zuoliangyu/EmbedKit/discussions)
- **邮件**: embedkit@example.com

---

<div align="center">

**[返回顶部](#embedkit)**

Made with ❤️ for embedded developers

</div>