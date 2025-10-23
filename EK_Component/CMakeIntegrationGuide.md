# EmbeddedKit CMake 集成指南

## 概述

EmbeddedKit (EK) 是一个轻量级的嵌入式组件库，提供了数据结构、任务调度、内存管理、协程系统等常用功能模块。本指南将详细介绍如何在您的项目中集成 EmbeddedKit。

## 目录结构

```
EK_Component/
├── CMakeLists.txt              # CMake 构建配置
├── cmake/                      # CMake 配置模板
│   └── EmbeddedKitConfig.cmake.in
├── EK_Common.h/.c              # 公共定义和工具函数
├── EK_Config.h                 # 配置文件
├── Bsp/                        # 板级支持包
│   ├── EK_Serial.c
│   └── Inc/EK_Serial.h
├── DataStruct/                 # 数据结构模块
│   ├── EK_List.c/.h
│   ├── EK_Queue.c/.h
│   └── EK_Stack.c/.h
├── MemPool/                    # 内存池模块
│   ├── EK_MemPool.c/.h
├── Task/                       # 任务调度模块
│   ├── EK_Task.c/.h
└── EK_Corotinue/               # 协程系统模块
    ├── Kernel.c/.h
    ├── EK_CoroTask.c/.h
    ├── EK_CoroMessage.c/.h
    ├── EK_CoroSemaphore.c/.h
    ├── Heap.c
    └── List.c
```

## 集成方式

### 方式一：作为子目录直接引用（推荐）

这是最简单直接的集成方式，适合快速开发和原型验证。

1. 将整个 `EK_Component` 文件夹复制到您的项目中（例如放在 `lib/EmbeddedKit`）
2. 在主工程的 `CMakeLists.txt` 中添加子目录：

```cmake
# 添加 EmbeddedKit 作为子目录
add_subdirectory(lib/EmbeddedKit)

# 链接到您的目标
target_link_libraries(YourApp PRIVATE EmbeddedKit::EmbeddedKit)
```

**优点：**
- 无需额外安装步骤
- 便于调试和修改源码
- 适合开发阶段

### 方式二：作为 Git 子模块引用

适合需要跟踪上游更新的项目。

1. 在您的项目根目录执行：

```bash
git submodule add https://github.com/your-repo/EmbeddedKit.git lib/EmbeddedKit
git submodule update --init --recursive
```

2. 在 `CMakeLists.txt` 中添加：

```cmake
add_subdirectory(lib/EmbeddedKit)
target_link_libraries(YourApp PRIVATE EmbeddedKit::EmbeddedKit)
```

### 方式三：安装后通过 find_package 引用

适合需要管理多个依赖的复杂项目或团队协作。

1. **安装 EmbeddedKit：**

```bash
cd EK_Component
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build --config Release
sudo cmake --install build --config Release
```

或在 Windows PowerShell 中：

```pwsh
cd EK_Component
cmake -S . -B build -DCMAKE_INSTALL_PREFIX="$env:USERPROFILE/.local"
cmake --build build --config Release
cmake --install build --config Release
```

2. **在消费工程中使用：**

```cmake
find_package(EmbeddedKit CONFIG REQUIRED)
target_link_libraries(YourApp PRIVATE EmbeddedKit::EmbeddedKit)
```

**安装后的变量：**
- `EmbeddedKit_INCLUDE_DIR`: 头文件目录路径
- `EmbeddedKit_VERSION`: 库版本信息

## 配置选项

EmbeddedKit 通过 `EK_Config.h` 提供丰富的配置选项。您可以根据项目需求定制功能模块：

### 主要功能开关

```c
// 数据结构模块总开关
#define EK_DATASTRUCT_ENABLE (1)

// 协程系统 vs 普通任务调度器
#define EK_CORO_ENABLE (1)     // 1: 协程系统, 0: 普通调度器

// 协程功能模块
#define EK_CORO_MESSAGE_QUEUE_ENABLE (1)
#define EK_CORO_SEMAPHORE_ENABLE (1)
#define EK_CORO_TASK_NOTIFY_ENABLE (1)
```

### 内存配置

```c
// 内存池配置
#define MEMPOOL_SIZE (1024 * 10)      // 总大小 10KB
#define MEMPOOL_ALIGNMENT (8)          // 8字节对齐
```

### 系统参数

```c
// 协程系统配置
#define EK_CORO_SYSTEM_FREQ (168000000)      // 系统时钟频率
#define EK_CORO_TICK_RATE_HZ (1000)          // 调度频率 1kHz
#define EK_CORO_PRIORITY_GROUPS (16)         // 优先级组数
```

## 使用示例

### 基本使用

```c
#include "EK_Common.h"
#include "EK_Config.h"
#include "DataStruct/Inc/EK_List.h"
#include "MemPool/Inc/EK_MemPool.h"

int main(void) {
    // 初始化内存池
    EK_Result result = EK_rMemPoolInit();
    if (result != EK_OK) {
        return -1;
    }

    // 创建链表
    EK_ListHandle_t list = EK_pListCreate();
    if (list == NULL) {
        return -1;
    }

    // 使用链表...

    return 0;
}
```

### 协程系统使用

```c
#include "EK_Common.h"
#include "EK_Corotinue/Inc/EK_CoroTask.h"
#include "EK_Corotinue/Inc/Kernel.h"

// 任务函数
void vTask1(void *pvParameters) {
    while (1) {
        // 任务逻辑
        EK_vTaskDelay(100);  // 延时 100ms
    }
}

void vTask2(void *pvParameters) {
    while (1) {
        // 任务逻辑
        EK_vTaskDelay(200);  // 延时 200ms
    }
}

int main(void) {
    // 创建任务
    EK_xTaskCreate(vTask1, "Task1", 128, NULL, 1, NULL);
    EK_xTaskCreate(vTask2, "Task2", 128, NULL, 2, NULL);

    // 启动调度器
    EK_vStartScheduler();

    return 0;
}
```

## 常见问题

### Q: 如何减少库的大小？

A: 通过 `EK_Config.h` 禁用不需要的功能模块：

```c
#define EK_DATASTRUCT_ENABLE (0)     // 禁用数据结构
#define EK_BSP_ENABLE (0)           // 禁用 BSP 模块
#define EK_CORO_MESSAGE_QUEUE_ENABLE (0)  // 禁用消息队列
```

### Q: 编译时出现未定义的符号？

A: 检查是否启用了相应的功能模块，确保链接了所有需要的源文件。

### Q: 如何自定义内存分配函数？

A: 在包含 EmbeddedKit 头文件之前定义宏：

```c
#define EK_MALLOC(x) my_malloc(x)
#define EK_FREE(x) my_free(x)

#include "EK_Common.h"
```

### Q: 协程和普通调度器如何选择？

A:
- **协程系统**：适合资源受限的单片机，提供轻量级的并发处理
- **普通调度器**：适合需要更复杂任务管理功能的场景

通过 `EK_CORO_ENABLE` 宏控制选择。

## 最佳实践

1. **配置优化**：根据项目需求调整 `EK_Config.h`，只启用需要的功能
2. **内存管理**：合理配置内存池大小，避免内存碎片
3. **任务优先级**：合理设置协程优先级，确保实时性要求
4. **错误处理**：始终检查 API 返回的 `EK_Result` 状态
5. **模块化使用**：只包含需要的头文件，减少编译依赖

## 技术支持

如遇到问题或需要技术支持，请：
1. 检查配置文件设置
2. 查看示例代码
3. 提交 Issue 到项目仓库

---

*EmbeddedKit - 让嵌入式开发更简单*