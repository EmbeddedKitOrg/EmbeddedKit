# EmbeddedKit - Test Branch

> **分支说明**: `test/nn` - EmbeddedKit 框架组件测试分支

## 测试架构概览

本分支包含 EmbeddedKit 嵌入式框架的完整测试环境，基于 STM32F407VGT6 微控制器构建。

## 项目结构

```
EmbeddedKit/
├── STM32F407VGT6_JLC_TKX/          # STM32F407 测试项目
│   ├── EK_Component/               # EmbeddedKit 框架核心
│   │   ├── MemPool/               # 内存池管理
│   │   ├── Task/                  # 任务调度器
│   │   ├── DataStruct/            # 数据结构 (List/Queue/Stack)
│   │   └── EK_Config.h            # 框架配置
│   ├── App/Src/                   # 测试应用层
│   │   ├── Test_MemPool.c         # 内存池测试
│   │   ├── Test_List.c            # 链表测试  
│   │   ├── Test_Queue.c           # 队列测试
│   │   ├── Test_Stack.c           # 栈测试
│   │   ├── Test_TaskSystem.c      # 任务系统测试
│   │   ├── Task_App.c             # 应用任务
│   │   └── Interrupt.c            # 中断处理
│   ├── Bsp/Src/                   # 板级支持
│   │   ├── Key_LED.c              # 按键LED驱动
│   │   └── Serial.c               # 串口通信
│   └── CMakeLists.txt             # 构建配置
└── LICENSE                        # MIT 许可证
```

## 测试组件

### 核心模块测试
- **内存池** - 动态内存分配、碎片管理、统计信息
- **任务调度器** - 优先级调度、任务生命周期、延时机制  
- **双向链表** - 节点操作、排序算法、内存管理
- **循环队列** - 环形缓冲、数据流控制
- **栈结构** - LIFO操作、容量管理

### 硬件抽象层
- **按键LED** - GPIO控制和状态指示
- **串口通信** - 调试输出和数据交互
- **中断处理** - 系统事件响应

## 构建环境

- **目标平台**: STM32F407VGT6
- **构建系统**: CMake
- **编译器**: GCC ARM
- **调试**: ST-Link

## 运行测试

```bash
cd STM32F407VGT6_JLC_TKX
mkdir build && cd build
cmake ..
make
```

## 测试验证

通过串口输出查看各组件测试结果，验证框架功能正确性和性能指标。

---

**作者**: N1netyNine99  
**许可证**: MIT