# L0_MCU (MCU厂商库层)

## 1. 简介
L0_MCU 是本项目的最底层，专门用于存放各 MCU 厂商提供的官方库和启动代码。

向上：为 L1_HAL 层提供底层的硬件操作接口（如寄存器定义、厂商 HAL/LL 库）。

本层是唯一与具体芯片型号强相关的层。更换 MCU 时，只需替换本层对应的内容。

## 2. 目录结构
```text
L0_MCU
├── CMakeLists.txt          # 构建脚本
├── stub
│   └── ek_app_stub.c      # 弱定义的函数入口 
├── STM32F429VGT6          # 具体的 MCU 型号目录
│   ├── Inc                # 厂商头文件
│   │   ├── stm32f4xx_hal_conf.h
│   │   ├── stm32f4xx_it.h
│   │   └── main.h         # 系统配置
│   ├── Src                # 厂商源文件
│   │   ├── stm32f4xx_hal_msp.c
│   │   ├── stm32f4xx_it.c
│   │   └── main.c         # 主程序入口（调用 ek_main）
│   ├── Drivers            # STM32 HAL 库
│   │   ├── STM32F4xx_HAL_Driver
│   │   └── CMSIS
│   └── startup            # 启动文件
│       └── startup_stm32f429xx.s
└── STM32F103C8T6          # 另一个 MCU 型号（示例）
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
当需要支持新的 MCU 型号时，在 L0_MCU 下创建对应的文件夹：

```text
L0_MCU/
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
// Src/main.c
#include "main.h"
#include "L5_App/app.h"

extern void ek_main(void); // 在开头声明可以避免 LSP 提示找不到 `ek_main`

int main(void)
{
    // 1. 系统初始化（时钟、配置等）
    SystemInit();
    HAL_Init();

    // 2. 调用上层业务入口
    ek_main();

    // 3. 正常情况下不会到达这里
    while(1) {}
}
```

### 步骤 4：更新 CMakeLists.txt
在 L0_MCU/CMakeLists.txt 中添加新的 MCU 子目录：

```cmake
# 根据项目配置选择当前使用的 MCU
if(USE_STM32F429)
    add_subdirectory(STM32F429VGT6)
elseif(USE_GD32F450)
    add_subdirectory(GD32F450VGT6)
endif()
```

## 5. 常见问题

**Q: 为什么不把所有厂商库放在同一个公共目录？**

A: 不同 MCU 厂商库的文件名可能相同（都是 stm32xxxx_hal.c），但内容不同。如果混在一起，会导致链接错误和版本混乱。每个 MCU 独立目录可以完全避免这个问题。

**Q: main.c 为什么放在 L0 而不是 L5？**

A: main.c 包含 MCU 特定的初始化代码（如 HAL_Init、SystemClock_Config），这些是与具体芯片相关的。L5 的 ek_main 应该是纯业务逻辑，与硬件无关。L0 的 main.c 充当了"胶水"代码的角色。

**Q: 如何在同一个工程中支持多个 MCU？**

A: 使用 CMake 的条件编译选项。在构建时指定 `-DUSE_STM32F429=ON` 或 `-DUSE_GD32F450=ON`，CMake 只会编译对应的 MCU 目录。
