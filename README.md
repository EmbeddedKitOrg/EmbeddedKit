# EmbeddedKit (EK)

一个专业的 6 层嵌入式固件框架，遵循严格的架构模式以实现最大模块化和硬件可移植性。使用基于 CMake 的构建系统，支持多种工具链。

[English Version](README_EN.md)

---

## 架构

EmbeddedKit 遵循严格的自下而上依赖模型，上层仅依赖下层。绝对禁止引入循环依赖。

```
L5_App (应用层)
  └─> 业务逻辑、状态机
L4_Components (设备驱动组件层)
  └─> 硬件驱动（OLED、Flash、传感器）- 使用 OOP 模式
L3_Middlewares (第三方中间件层)
  └─> FreeRTOS、FatFs、LVGL 等
L2_Core (核心与硬件抽象层)
  ├─> utils/ (数据结构、内存管理、日志)
  └─> hal/ (硬件抽象层：GPIO/UART/I2C 等)
L1_MCU (MCU 厂商库层)
  └─> 芯片特定的厂商代码（STM32 HAL、CMSIS）
L0_Assets (资源文件层)
  └─> 静态资源数据（图片、字体、配置等）
```

### 关键架构规则

- **L0_Assets（最底层）**：存放编译时嵌入的静态资源数据（图片、字体、配置等）。不依赖任何其他层级，可被任何层访问。

- **L1_MCU（MCU 厂商库层）**：每个 MCU 型号拥有独立子目录，包含厂商 HAL/LL 库、启动代码和 `main.c`。`main.c` 在硬件初始化后调用 L5_App 的 `ek_main()`。禁止跨不同 MCU 共享厂商库。

- **L2_Core**：
  - **utils/**：纯软件实现 - 数据结构、内存管理、日志
  - **hal/**：硬件抽象层，提供逻辑到物理的映射
    - `inc/*.h` 绝对不能包含厂商头文件（stm32xxxx.h）
    - 厂商头文件仅允许在 `src/*.c` 中包含
    - 所有函数使用 `hal_` 前缀（如 `hal_gpio_write()`）
  - 内部依赖：hal/ 可以依赖 utils/（同层内部允许）
  - 命名规范：`ek_` 前缀（utils）和 `hal_` 前缀（hal）

- **L3_Middlewares**：每个中间件拥有独立子目录和独立的 CMakeLists.txt。可依赖 L2_Core/hal 进行硬件适配。

- **L4_Components**：**严格 OOP 模式** - 使用函数指针实现多态。方法的第一个参数必须是对象指针（`self`）。定义抽象接口实现硬件依赖倒置。**由用户根据实际硬件自行实现**。

- **L5_App**：实现 `ek_main()` 作为应用入口点。应调用 L4 组件，而非直接调用 L1。将业务逻辑封装到模块中。

---

## 特性

- **6 层架构**：清晰的职责分离，严格的依赖规则
- **对象库构建模式**：使用 OBJECT 库避免静态库的选择性链接问题
- **硬件可移植性**：更换 MCU 只需替换 L1 层
- **OOP 设计模式**：L4 层使用接口抽象实现依赖倒置
- **灵活的构建系统**：支持多种工具链（GCC、ARM Compiler 6、Clang）
- **条件编译**：通过 CMake 选项启用/禁用功能
- **丰富的数据结构**：链表、环形缓冲区、栈、动态向量（已包含）
- **内存管理**：基于 TLSF 的动态内存分配器
- **日志系统**：多级日志，支持彩色输出
- **RTOS 支持**：FreeRTOS 集成就绪
- **资源管理**：独立的资源文件层，支持静态数据嵌入

---

## 快速开始

### 前置要求

- CMake 3.20 或更高版本
- ARM 工具链（gcc-arm-none-eabi 或 STARM Clang）
- STM32CubeMX 或 GD32 Eclipse（用于生成初始化代码）
- [just](https://github.com/casey/just)（可选，提供更简洁的构建命令）

### 构建

**方式一：使用 just（推荐）**

```bash
# 构建 STM32F407VGT6（GCC ARM）
just build

# 构建 GD32F470ZGT6（GCC ARM）
just build-gd

# 构建 STM32F429ZIT6（STARM Clang）
just build-starm

# 清理构建目录
just clean

# 运行单元测试
just test
```

**方式二：使用 CMake**

```bash
# 配置构建（通过缓存变量选择 MCU 型号）
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake \
  -DMCU_MODEL=STM32F407VGT6_GCC \
  -DUSE_FREERTOS=OFF \
  -DUSE_FATFS=OFF \
  -DUSE_LVGL=OFF

# 构建
cmake --build build
```

### 可用工具链

- `cmake/gcc-arm-none-eabi.cmake` - GCC ARM 工具链
- `cmake/starm-clang.cmake` - STARM Clang 工具链

**注意**：如需使用 ARM Compiler 6，请自行编写工具链文件。

---

## 目录结构

```
EmbeddedKit/
├── CMakeLists.txt              # 主构建脚本
├── ek_conf.h                   # 全局配置文件
├── CLAUDE.md                   # 项目文档和代码修改记录
├── LICENSE                     # MIT 许可证
├── cmake/                      # 工具链文件目录
│   ├── gcc-arm-none-eabi.cmake
│   ├── arm-compile6.cmake
│   └── starm-clang.cmake
├── L0_Assets/                  # 第0层：资源文件
├── L1_MCU/                     # 第1层：MCU 厂商库
│   └── STM32F429ZIT6_GCC/      # STM32F429 支持
├── L2_Core/                    # 第2层：核心与硬件抽象层
│   ├── utils/                  # 纯软件工具库
│   ├── hal/                    # 硬件抽象层
│   └── third_party/            # 第三方库
├── L3_Middlewares/             # 第3层：中间件
│   ├── FreeRTOS/               # 实时操作系统
│   ├── FatFS/                  # 文件系统
│   └── LVGL/                   # 图形库
├── L4_Components/              # 第4层：设备驱动（OOP）
└── L5_App/                     # 第5层：应用逻辑
```

---

## 各层说明

### L0_Assets（资源文件层）

存放编译时嵌入的静态资源数据，不依赖任何其他层级。

**用途**：图片、字体、配置数据等静态资源

**特点**：
- 资源以 C 数组或 const 数据形式提供
- 直接链接到最终固件
- 可被任何层访问

### L1_MCU（MCU 厂商库层）

存放各 MCU 厂商提供的官方库和启动代码。每个 MCU 型号拥有独立子目录。`main.c` 负责在硬件初始化后调用 `ek_main()`。

**当前支持**：STM32F407VGT6_GCC、STM32F429ZIT6_STARM、GD32F470ZGT6

**构建模式**：OBJECT 库，通过 `$<TARGET_OBJECTS:l1_mcu>` 链接到最终固件

### L2_Core（核心与硬件抽象层）

**utils/（硬件无关）**：
- `ek_def.h` - 通用定义和跨编译器宏
- `ek_list.h` - 双向循环链表
- `ek_ringbuf.h` - 环形缓冲区（带元素计数）
- `ek_stack.h` - 栈数据结构
- `ek_vec.h` - 动态数组（Vector）
- `ek_mem.h` - 基于 TLSF 的动态内存管理
- `ek_log.h` - 分级日志系统
- `ek_assert.h` - 断言模块
- `ek_export.h` - 导出宏（自动初始化）
- `ek_io.h` - IO 模块（基于 lwprintf）
- `ek_shell.h` - Shell 模块
- `ek_str.h` - 字符串处理工具

**hal/（硬件抽象）**：实现中。提供逻辑到物理的映射（如 `HAL_GPIO_1` → `GPIOA PIN_5`）。
- GPIO、UART、SPI、I2C、Tick

**third_party/**：
- `lwprintf` - 轻量级格式化输出库
- `tlsf` - Two-Level Segregated Fit 内存分配器

### L3_Middlewares（第三方中间件层）

每个中间件拥有独立子目录和 CMakeLists.txt。

**已集成的中间件**：

1. **FreeRTOS** - 实时操作系统（✅ 完全支持）
   - 端口：GCC_ARM_CM4F（对应 STM32F429）
   - 堆实现：heap_4.c
   - 配置：168MHz CPU、1000Hz Tick Rate、32KB 堆
   - 启用方式：`-DUSE_FREERTOS=ON`

2. **FatFS** - 文件系统（✅ 完全支持）
   - 启用方式：`-DUSE_FATFS=ON`

3. **LVGL** - 轻量级图形库（✅ 完全支持）
   - 版本：v8.3.11
   - 配置：RGB565、128KB 内存池
   - 包含：官方 examples 和 demos
   - 启用方式：`-DUSE_LVGL=ON`
   - **注意**：LVGL 链接了 L1_MCU，可直接访问硬件相关功能

### L4_Components（硬件驱动组件层）

**核心设计 - OOP 模式**：
- 使用函数指针定义抽象接口
- 使用结构体封装属性和方法
- 依赖倒置 - 组件依赖抽象接口
- 多态支持 - 同一组件支持多种硬件

**实现方式**：**由用户根据实际硬件自行实现**

### L5_App（应用层）

实现 `ek_main()` 作为应用入口点。应调用 L2~L4 层提供的服务，而非直接调用 L1。

**当前实现**：
```c
void ek_main(void)
{
    ek_heap_init();  // 初始化内存堆

    while (1)
    {
        // 业务逻辑
    }
}
```

---

## 配置

根目录的全局配置文件 `ek_conf.h` 用于管理框架级别的设置：

```c
// RTOS 配置
#define EK_USE_RTOS (1)

// 内存管理
#define EK_HEAP_NO_TLSF (0)
#define EK_HEAP_SIZE (30 * 1024)

// IO库管理
#define EK_IO_NO_LWPRTF   (1)

// 模块功能开关
#define EK_EXPORT_ENABLE  (0)
#define EK_STR_ENABLE     (1)
#define EK_LOG_ENABLE     (1)
#define EK_LIST_ENABLE    (1)
#define EK_VEC_ENABLE     (1)
#define EK_RINGBUF_ENABLE (1)
#define EK_STACK_ENABLE   (1)
#define EK_SHELL_ENABLE   (1)

// 日志模块配置
#define EK_LOG_DEBUG_ENABLE (1)
#define EK_LOG_COLOR_ENABLE (1)
#define EK_LOG_BUFFER_SIZE  (256)

// 断言模块配置
#define EK_ASSERT_USE_TINY (1)
#define EK_ASSERT_WITH_LOG (1)
```

---

## 构建系统

### 对象库（OBJECT Library）架构

本项目使用 **OBJECT 库**模式而非传统的静态库模式，具有以下优势：

**为什么使用 OBJECT 库？**

传统静态库（STATIC Library）存在选择性链接问题：
- 链接器只提取能解析当前未定义引用的目标文件
- 被"跳过"的符号不会被包含在最终固件中
- 需要使用 `--whole-archive`、`--undefined` 等复杂选项来强制包含符号

OBJECT 库的优势：
- 对象文件直接参与最终链接，不存在选择性链接问题
- 无需 `--whole-archive`、`--undefined` 等链接器选项
- 构建系统更简洁，符号解析更可靠

**CMake 实现方式：**

```cmake
# 各层定义为 OBJECT 库
add_library(l0_assets OBJECT)
add_library(l1_mcu OBJECT)
add_library(l2_core OBJECT)
add_library(l4_components OBJECT)
add_library(l5_app OBJECT)

# L3 保持 INTERFACE 库（聚合层）
add_library(l3_middlewares INTERFACE)

# 最终链接使用 $<TARGET_OBJECTS:>
target_link_libraries(${CMAKE_PROJECT_NAME}
    $<TARGET_OBJECTS:l5_app>
    $<TARGET_OBJECTS:l4_components>
    l3_middlewares                      # INTERFACE 库正常链接
    $<TARGET_OBJECTS:l2_core>
    $<TARGET_OBJECTS:l1_mcu>
    $<TARGET_OBJECTS:l0_assets>
)
```

---

## 项目状态

### 已完成
- ✅ 完整的 6 层架构设计
- ✅ CMake 构建系统配置
- ✅ just 命令行工具支持
- ✅ L0_Assets：资源文件层框架就绪
- ✅ L1_MCU 层：STM32F407VGT6_GCC、STM32F429ZIT6_STARM、GD32F470ZGT6 支持完整
- ✅ L2_Core/utils 层：完整实现所有工具模块（包含 export、io、shell、str 等 12 个模块）
- ✅ L2_Core/third_party：集成 lwprintf 和 tlsf
- ✅ L2_Core/hal 层：GPIO、UART、SPI、I2C、Tick 实现中
- ✅ L3_Middlewares：FreeRTOS、FatFS、LVGL v8.3.11 完全支持（包含 examples 和 demos）
- ✅ L5_App：基础入口实现
- ✅ 全局配置文件 `ek_conf.h`
- ✅ 详细的文档

### 待实现
- ⏳ L2_Core/hal 层：完成剩余外设（DAC、ADC、RTC 等）
- ⏳ L4_Components 层：由用户根据实际硬件自行实现设备驱动组件
- ⏳ L5_App 层：具体的业务逻辑应用

---

## 贡献

欢迎贡献！请遵循本文档描述的架构规则和编码标准。

---

## 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件。
