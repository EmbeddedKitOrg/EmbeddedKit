# L1_MCU (MCU厂商库层)

## 1. 简介
L1_MCU 是本项目的最底层，专门用于存放各 MCU 厂商提供的官方库和启动代码。

向上：为 L2_Core/hal 层提供底层的硬件操作接口（如寄存器定义、厂商 HAL/LL 库）。

本层是唯一与具体芯片型号强相关的层。更换 MCU 时，只需替换本层对应的内容。

## 2. 目录结构
```text
L1_MCU
├── CMakeLists.txt          # 构建脚本
├── STM32F429ZIT6_GCC      # 具体的 MCU 型号目录（GCC 工具链）
│   ├── Core/Inc           # 厂商头文件
│   │   ├── stm32f4xx_hal_conf.h
│   │   ├── stm32f4xx_it.h
│   │   └── main.h         # 系统配置
│   ├── Core/Src           # 厂商源文件
│   │   ├── stm32f4xx_hal_msp.c
│   │   ├── stm32f4xx_it.c
│   │   ├── syscalls.c     # 系统调用（_sbrk, _write, _read 等）
│   │   └── main.c         # 主程序入口（调用 ek_main）
│   ├── Drivers            # STM32 HAL 库
│   │   ├── STM32F4xx_HAL_Driver
│   │   └── CMSIS
│   ├── startup            # 启动文件
│   │   └── startup_stm32f429xx.s
│   └── stm32f429xx_flash.ld  # 链接脚本
└── STM32F407VGT6_GCC      # 另一个 MCU 型号
    └── ...
```

## 3. 核心开发原则 (Strict Rules)
为了保证架构的整洁性，开发本层时必须遵守以下规则：

**单 MCU 单目录**：

每个具体的 MCU 型号（如 STM32F429VGT6、STM32F103C8T6）必须拥有自己独立的文件夹。

不同 MCU 之间的代码完全隔离，互不干扰。

**厂商库隔离**：

厂商提供的官方库（如 STM32 HAL、LL 库）必须存放在对应 MCU 的目录下。

禁止跨 MCU 共享厂商库文件，避免版本冲突。

**启动代码管理**：

启动文件（startup_xxxx.s）和链接脚本（LD脚本）应与 MCU 型号绑定。

main.c 文件负责系统初始化，然后调用 L5_App 层的 `ek_main()` 函数。

## 4. 开发指南

### 步骤 1：创建新的 MCU 目录
当需要支持新的 MCU 型号时，在 L1_MCU 下创建对应的文件夹：

```text
L1_MCU/
└── GD32F450VGT6/          # 新增 MCU
    ├── Inc/
    ├── Src/
    ├── Drivers/           # GD32 固件库
    └── startup/
```

### 步骤 2：添加厂商库
将 MCU 厂商提供的固件库复制到对应的 Drivers 目录：

```text
GD32F450VGT6/
└── Drivers/
    ├── GD32F4xx_standard_peripheral/
    └── CMSIS/
```

### 步骤 3：编写 main.c 入口
在 main.c 中完成硬件初始化后，调用上层业务入口：

```c
// Core/Src/main.c
#include "main.h"

extern void ek_main(void);  // L5_App 层的应用入口

int main(void)
{
    // 1. 系统初始化（时钟、配置等）
    HAL_Init();
    SystemClock_Config();

    // 2. 调用上层业务入口
    ek_main();

    // 3. 正常情况下不会到达这里
    while(1) {}
}
```

**跨层调用机制说明**：

L1 层的 `main.c` 需要调用 L5 层的 `ek_main()`，这违反了"上层依赖下层"的常规规则。为解决此问题，项目采用链接器选项强制符号解析：

```cmake
# 根目录 CMakeLists.txt
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    "-Wl,--undefined=ek_main"  # 强制链接器从 L5_App 中提取 ek_main
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    l5_app                      # L5 层必须提供 ek_main 强定义
    l4_components
    l3_middlewares
    l2_core
    "-Wl,--whole-archive" l1_mcu "-Wl,--no-whole-archive"  # 包含 syscalls
    l0_assets
)
```

### 步骤 4：更新 CMakeLists.txt
在 L1_MCU/CMakeLists.txt 中使用变量选择 MCU 型号：

```cmake
# 设置默认 MCU 型号，允许命令行 -DMCU_MODEL=xxx 覆盖
set(MCU_MODEL "STM32F429ZIT6_GCC" CACHE STRING "Select MCU Model")

# 进入对应的 MCU 子目录
add_subdirectory(${MCU_MODEL})
```

构建时指定 MCU 型号：
```bash
cmake -B build -DMCU_MODEL=STM32F429ZIT6_GCC
cmake -B build -DMCU_MODEL=STM32F407VGT6_GCC
```

## 5. 常见问题

**Q: 为什么不把所有厂商库放在同一个公共目录？**

A: 不同 MCU 厂商库的文件名可能相同（都是 stm32xxxx_hal.c），但内容不同。如果混在一起，会导致链接错误和版本混乱。每个 MCU 独立目录可以完全避免这个问题。

**Q: main.c 为什么放在 L1 而不是 L5？**

A: main.c 包含 MCU 特定的初始化代码（如 HAL_Init、SystemClock_Config），这些是与具体芯片相关的。L5 的 ek_main 应该是纯业务逻辑，与硬件无关。L1 的 main.c 充当了"胶水"代码的角色。

**Q: L1 调用 L5 的 ek_main 会不会破坏分层架构？**

A: 这是嵌入式系统的特殊情况。C 标准要求程序从 main 函数开始，而 main 必须在 L1 层（因为需要硬件初始化）。通过 `--undefined=ek_main` 链接器选项，我们在不改变各层静态库属性的前提下实现了这种"反向调用"。L5 层仍是独立的静态库，可以在其他项目中复用。

**Q: 为什么需要 --whole-archive 包含 l1_mcu？**

A: libc_nano.a 需要系统调用符号（`_sbrk`, `_write`, `_read` 等），这些符号定义在 L1_MCU 的 syscalls.c 中。由于静态库的选择性链接机制，如果 l1_mcu 没有被引用，这些符号会被跳过。`--whole-archive` 强制包含 l1_mcu 的所有符号。

**Q: 如何在同一个工程中支持多个 MCU？**

A: 使用 CMake 的 `MCU_MODEL` 变量。在构建时指定 `-DMCU_MODEL=STM32F429ZIT6_GCC` 或 `-DMCU_MODEL=STM32F407VGT6_GCC`，CMake 只会编译对应的 MCU 目录。

