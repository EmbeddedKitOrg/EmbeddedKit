# L3_Middlewares (中间件集成层)

## 1. 简介
L3_Middlewares 是本项目的第三方中间件集成层，用于管理和集成各种成熟的第三方库。

向下：可能依赖 L1_HAL 的硬件驱动（如 FreeRTOS 依赖定时器、LVGL 依赖显示接口）。

向上：为 L4~L5 层提供高级功能（如实时任务调度、文件系统、图形界面等）。

本层的设计原则是**组件化隔离**，每个第三方库都独立管理，便于版本升级和替换。

## 2. 目录结构
```text
L3_Middlewares
├── CMakeLists.txt              # 主构建脚本
├── FreeRTOS                    # FreeRTOS 实时操作系统
│   ├── CMakeLists.txt          # FreeRTOS 子模块构建
│   ├── Inc                     # 配置头文件
│   │   └── FreeRTOSConfig.h    # RTOS 配置
│   └── Src                     # FreeRTOS 源码
│       ├── croutine.c
│       ├── tasks.c
│       └── ...
├── FatFs                       # FatFs 文件系统
│   ├── CMakeLists.txt          # FatFs 子模块构建
│   └── Src                     # FatFs 源码
│       ├── ff.c
│       ├── ffconf.h            # FatFs 配置
│       └── diskio.h            # 磁盘接口（需实现）
└── LVGL                        # LVGL 图形库
    ├── CMakeLists.txt          # LVGL 子模块构建
    └── Src                     # LVGL 源码
        ├── lvgl/
        └── lv_conf.h           # LVGL 配置
```

## 3. 核心开发原则 (Strict Rules)
为了保证架构的整洁性和可维护性，开发本层时必须遵守以下规则：

**组件隔离 (Component Isolation)**：

每个第三方库必须拥有自己独立的子目录。

每个组件都有自己的 CMakeLists.txt，负责编译该组件的源文件。

**统一构建管理**：

主 CMakeLists.txt 通过 `add_subdirectory()` 集成各组件。

允许通过条件编译选择性启用或禁用某个组件。

**版本控制建议**：

建议使用 Git Submodule 管理第三方库，方便追踪版本更新。

每个组件的 README 中记录当前使用的版本号和来源。

## 4. 开发指南

### 步骤 1：添加新的第三方库
在 L3_Middlewares 下创建新的子目录：

```bash
L3_Middlewares/
└── cJSON/                      # 新增 JSON 解析库
    ├── CMakeLists.txt
    ├── cJSON.h
    └── cJSON.c
```

### 步骤 2：编写组件的 CMakeLists.txt
为每个组件编写独立的构建脚本：

```cmake
# L3_Middlewares/cJSON/CMakeLists.txt

# 收集源文件
set(CJSON_SRC
    cJSON.c
)

# 创建静态库
add_library(cJSON STATIC ${CJSON_SRC})

# 设置包含目录
target_include_directories(cJSON PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 如果需要编译选项
target_compile_options(cJSON PRIVATE
    -std=c99
)
```

### 步骤 3：在主 CMakeLists.txt 中注册
在 L3_Middlewares/CMakeLists.txt 中添加子模块：

```cmake
# L3_Middlewares/CMakeLists.txt

# 根据配置选项选择性添加组件
option(USE_FREERTOS "Enable FreeRTOS" ON)
option(USE_FATFS "Enable FatFs" ON)
option(USE_LVGL "Enable LVGL" OFF)

if(USE_FREERTOS)
    add_subdirectory(FreeRTOS)
endif()

if(USE_FATFS)
    add_subdirectory(FatFs)
endif()

if(USE_LVGL)
    add_subdirectory(LVGL)
endif()

# 新增的 cJSON
add_subdirectory(cJSON)
```

### 步骤 4：实现硬件适配层
某些中间件需要与 L1_HAL 对接，实现适配接口：

```c
// FatFs 磁盘 I/O 适配示例 (L3_Middlewares/FatFs/Src/diskio.c)
#include "ff.h"
#include "hal_spi.h"     // L1 层的 SPI 驱动

DSTATUS disk_initialize(BYTE pdrv)
{
    // 初始化 SD 卡的 SPI 接口
    hal_spi_init(HAL_SPI_SD_CARD);
    return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
{
    // 调用 L1 层的 SPI 发送函数
    hal_spi_send_blocks(HAL_SPI_SD_CARD, buff, sector, count);
    return RES_OK;
}
```

### 步骤 5：配置文件管理
每个中间件都有自己的配置文件，需要根据项目调整：

```c
// FreeRTOSConfig.h 示例
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#define configCPU_CLOCK_HZ          168000000UL
#define configTICK_RATE_HZ          1000
#define configTOTAL_HEAP_SIZE       (32 * 1024)
#define configMAX_PRIORITIES        5
#define configUSE_PREEMPTION        1

#endif
```

```c
// lv_conf.h 示例
#define LV_COLOR_DEPTH          16
#define LV_HOR_RES_MAX          320
#define LV_VER_RES_MAX          240
#define LV_USE_FREETYPE         0
```

## 5. 常见问题

**Q: 如何更新第三方库到新版本？**

A: 如果使用 Git Submodule，可以使用 `git submodule update` 更新。如果是手动复制的库，只需替换对应目录下的源文件，然后检查配置文件（如 lv_conf.h）是否有变化。

**Q: 某个中间件不需要了，如何移除？**

A: 只需在 L3_Middlewares/CMakeLists.txt 中注释掉对应的 `add_subdirectory()`，然后在项目配置中禁用相关选项（如 `set(USE_LVGL OFF)`）。编译器将不会编译该组件的代码。

