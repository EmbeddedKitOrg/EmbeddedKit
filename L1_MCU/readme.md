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

**跨层调用机制说明（OBJECT 库架构）**：

L1 层的 `main.c` 需要调用 L5 层的 `ek_main()`，这违反了"上层依赖下层"的常规规则。

**旧方案（STATIC 库 + 链接器选项）**：
需要使用 `--undefined=ek_main` 强制符号解析，以及 `--whole-archive` 包含系统调用符号。

**新方案（OBJECT 库）**：
通过使用 OBJECT 库，对象文件直接参与最终链接，不存在选择性链接问题：

```cmake
# 各层定义为 OBJECT 库
add_library(l0_assets OBJECT)
add_library(l1_mcu OBJECT)
add_library(l2_core OBJECT)
add_library(l4_components OBJECT)
add_library(l5_app OBJECT)

# 最终链接使用 $<TARGET_OBJECTS:>
target_link_libraries(${CMAKE_PROJECT_NAME}
    $<TARGET_OBJECTS:l5_app>         # ek_main 符号自动包含
    $<TARGET_OBJECTS:l4_components>
    l3_middlewares                   # INTERFACE 库正常链接
    $<TARGET_OBJECTS:l2_core>
    $<TARGET_OBJECTS:l1_mcu>         # syscalls 符号自动包含
    $<TARGET_OBJECTS:l0_assets>
)
```

OBJECT 库的优势：
- 无需 `--undefined=ek_main`（符号自动包含）
- 无需 `--whole-archive`（所有对象文件都参与链接）
- 构建系统更简洁，符号解析更可靠

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

### 步骤 5：编写 MCU 子目录的 CMakeLists.txt

每个 MCU 子目录需要自己的 `CMakeLists.txt`，用于组织该 MCU 的源文件、头文件和厂商库。

**标准模板（参考 STM32F407VGT6_GCC/CMakeLists.txt）：**

```cmake
# =============================================================================
# 1. 头文件路径
# =============================================================================
set(MX_Include_Dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc           # 用户代码头文件
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/ARM/DSP/Inc  # 可选
)

# =============================================================================
# 2. 收集源文件
# =============================================================================
file(GLOB MX_Start_S ${CMAKE_CURRENT_SOURCE_DIR}/*.s)              # 启动文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Core/Src MX_Core_Src)    # 用户代码
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src MX_Driver_Src)  # HAL 库

# =============================================================================
# 3. 创建 OBJECT 库（整合到 l1_mcu）
# =============================================================================
add_library(cubemx_hal OBJECT ${MX_Start_S} ${MX_Core_Src} ${MX_Driver_Src})

# ARM DSP 数学库（可选，有则添加）
target_sources(cubemx_hal PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/ARM/DSP/Lib/libarm_cortexM4lf_math.a
)

# 头文件路径
target_include_directories(cubemx_hal PUBLIC ${MX_Include_Dirs})

# STM32 宏定义（根据具体型号修改）
target_compile_definitions(cubemx_hal PUBLIC
    USE_HAL_DRIVER
    STM32F407xx    # F407 用 STM32F407xx，F429 用 STM32F429xx
    $<$<CONFIG:Debug>:DEBUG>
)

# 链接全局配置
target_link_libraries(cubemx_hal PUBLIC global_macros global_options)

# =============================================================================
# 4. 整合到父层 l1_mcu
# =============================================================================
# 将对象文件添加到 l1_mcu
target_sources(l1_mcu PRIVATE $<TARGET_OBJECTS:cubemx_hal>)

# 传递头文件路径给 l1_mcu 的使用者
target_include_directories(l1_mcu PUBLIC $<TARGET_PROPERTY:cubemx_hal,INTERFACE_INCLUDE_DIRECTORIES>)

# 传递宏定义给 l1_mcu 的使用者
target_compile_definitions(l1_mcu PUBLIC $<TARGET_PROPERTY:cubemx_hal,INTERFACE_COMPILE_DEFINITIONS>)
```

**关键修改点：**

| 项目 | 说明 |
|------|------|
| `MX_Include_Dirs` | 根据实际目录结构调整，厂商库路径需匹配 |
| `STM32F407xx` | F407 用 `STM32F407xx`，F429 用 `STM32F429xx`，F103 用 `STM32F103xx` |
| `libarm_cortexM4lf_math.a` | 只有 M4 内核才有，M3/M0 不需要 |
| `cubemx_hal` | 名称可自定义，建议统一便于识别 |

## 5. 常见问题

**Q: 为什么不把所有厂商库放在同一个公共目录？**

A: 不同 MCU 厂商库的文件名可能相同（都是 stm32xxxx_hal.c），但内容不同。如果混在一起，会导致链接错误和版本混乱。每个 MCU 独立目录可以完全避免这个问题。

**Q: main.c 为什么放在 L1 而不是 L5？**

A: main.c 包含 MCU 特定的初始化代码（如 HAL_Init、SystemClock_Config），这些是与具体芯片相关的。L5 的 ek_main 应该是纯业务逻辑，与硬件无关。L1 的 main.c 充当了"胶水"代码的角色。

**Q: L1 调用 L5 的 ek_main 会不会破坏分层架构？**

A: 这是嵌入式系统的特殊情况。C 标准要求程序从 main 函数开始，而 main 必须在 L1 层（因为需要硬件初始化）。使用 OBJECT 库架构后，对象文件直接参与链接，不需要复杂的链接器选项。L5 层的 `ek_main` 符号会自动被包含，L5 层仍是独立的模块，可以在其他项目中复用。

**Q: 为什么不再需要 --whole-archive？**

A: 旧方案使用 STATIC 库时，libc_nano.a 需要系统调用符号（`_sbrk`, `_write`, `_read` 等），这些符号定义在 L1_MCU 的 syscalls.c 中。由于静态库的选择性链接机制，如果不使用 `--whole-archive`，这些符号会被跳过。

新方案使用 OBJECT 库后，所有对象文件都直接参与最终链接，不存在选择性链接问题，因此不再需要 `--whole-archive`。

**Q: 如何在同一个工程中支持多个 MCU？**

A: 使用 CMake 的 `MCU_MODEL` 变量。在构建时指定 `-DMCU_MODEL=STM32F429ZIT6_GCC` 或 `-DMCU_MODEL=STM32F407VGT6_GCC`，CMake 只会编译对应的 MCU 目录。

