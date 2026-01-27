# L0_Assets (资源文件层)

## 1. 简介
L0_Assets 是本项目的资源文件层，用于存放与固件一起打包的资源数据。

向上：为各层提供资源数据访问接口。

本层主要用于存储不需要编译的静态资源，如：
- 图标、图片数据
- 字体文件
- 配置数据
- 其他二进制资源

## 2. 目录结构
```text
L0_Assets
├── CMakeLists.txt          # 构建脚本
├── inc/                    # 头文件目录（资源声明）
│   └── assets.h
└── src/                    # 源文件目录（资源定义）
    └── assets.c
```

## 3. 核心开发原则

**静态资源管理**：

本层主要存放编译到固件中的静态资源数据。

资源以 C 数组或 const 数据的形式提供，直接链接到最终固件。

**无依赖设计**：

本层不依赖任何其他层级（L1~L5），仅依赖 `global_macros` 和 `global_options`。

保持最底层的独立性，确保资源数据可被任何层访问。

## 4. 使用示例

### 添加图片资源

```c
// inc/logo.h
#ifndef LOGO_H
#define LOGO_H

#include <stdint.h>

// Logo 图片数据 (RGB565 格式)
extern const uint16_t logo_data[];
extern const uint32_t logo_width;
extern const uint32_t logo_height;

#endif
```

```c
// src/logo.c
#include "logo.h"

const uint16_t logo_data[] = {
    0xFFFF, 0xFFFF, 0x0000, // ...
};

const uint32_t logo_width = 64;
const uint32_t logo_height = 64;
```

### 在上层使用资源

```c
// L5_App 或 L4 组件中使用
#include "assets.h"

void display_logo(void)
{
    // 使用资源数据
    lcd_draw_bitmap(0, 0, logo_width, logo_height, logo_data);
}
```

## 5. 常见问题

**Q: 为什么需要单独的资源层？**

A: 将资源数据集中管理，便于维护和复用。同时，资源数据可以作为独立模块被任何层访问，符合分层架构的设计理念。

**Q: 资源文件如何影响固件大小？**

A: 所有资源数据都会被链接到最终固件中，增加 FLASH 占用。建议使用压缩格式或优化资源大小。

**Q: 能否支持外部资源文件？**

A: 本层仅支持编译时嵌入的静态资源。如需运行时加载外部文件（如 SD 卡），应在 L4_Components 中实现文件系统驱动。
