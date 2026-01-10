# EmbeddedKit (EK)

A professional 5-layer embedded firmware framework designed for maximum modularity and hardware portability. Targeted primarily at ARM Cortex-M microcontrollers (default configuration: STM32F429), using a CMake-based build system with support for multiple toolchains.

[中文版](README_CN.md)

---

## Architecture

EmbeddedKit follows a strict bottom-up dependency model where upper layers depend only on lower layers. Circular dependencies are strictly prohibited.

```
L5_App (Application Layer)
  └─> Business logic, state machines
L4_Components (Device Driver Component Layer)
  └─> Hardware drivers (OLED, Flash, sensors) - Using OOP pattern
L3_Middlewares (Third-party Middleware Layer)
  └─> FreeRTOS, FatFs, LVGL, etc.
L2_Core (Core & Hardware Abstraction Layer)
  ├─> utils/ (Data structures, memory management, logging)
  └─> hal/ (Hardware abstraction: GPIO/UART/I2C/etc.)
L1_MCU (MCU Vendor Library Layer)
  └─> Chip-specific vendor code (STM32 HAL, CMSIS)
```

### Key Architectural Rules

- **L1_MCU (Lowest Layer)**: Each MCU model has an independent subdirectory containing vendor HAL/LL libraries, startup code, and `main.c`. The `main.c` calls `ek_main()` from L5_App after hardware initialization. No sharing of vendor libraries across different MCUs.

- **L2_Core**:
  - **utils/**: Pure software implementations - data structures, memory management, logging
  - **hal/**: Hardware abstraction layer providing logical-to-physical mapping
    - `inc/*.h` must NOT include vendor headers (stm32xxxx.h)
    - Vendor headers only allowed in `src/*.c`
    - All functions use `hal_` prefix (e.g., `hal_gpio_write()`)
  - Internal dependency: hal/ may depend on utils/ (allowed within same layer)
  - Naming convention: `ek_` prefix (utils) and `hal_` prefix (hal)

- **L3_Middlewares**: Each middleware has an independent subdirectory with its own CMakeLists.txt. Can depend on L2_Core/hal for hardware adaptation.

- **L4_Components**: **Strict OOP Pattern** - Use function pointers for polymorphism. The first parameter of methods must be the object pointer (`self`). Define abstract interfaces to implement dependency inversion for hardware.

- **L5_App**: Implements `ek_main()` as the application entry point. Should call L4 components, not L1 directly. Encapsulate business logic into modules.

---

## Features

- **5-Layer Architecture**: Clear separation of concerns with strict dependency rules
- **Hardware Portability**: Replace MCU by only changing the L1 layer
- **OOP Design Pattern**: Interface abstraction in L4 for dependency inversion
- **Flexible Build System**: Support for multiple toolchains (GCC, ARM Compiler 6, Clang)
- **Conditional Compilation**: Enable/disable features via CMake options
- **Rich Data Structures**: Linked list, ring buffer, stack, dynamic vector (included)
- **Memory Management**: TLSF-based dynamic memory allocator
- **Logging System**: Multi-level logging with color support
- **RTOS Support**: FreeRTOS integration ready

---

## Quick Start

### Prerequisites

- CMake 3.20 or higher
- ARM toolchain (gcc-arm-none-eabi, ARM Compiler 6, or clang)
- STM32CubeMX (for generating initialization code)

### Build

```bash
# Configure build (select MCU model via cache variable)
cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake -DMCU_MODEL=STM32F429ZIT6

# Build
cmake --build build

# Output files: EK_DEMO.elf, EK_DEMO.hex, EK_DEMO.bin, EK_DEMO.map
```

### Available Toolchains

- `cmake/gcc-arm-none-eabi.cmake` - GCC ARM toolchain
- `cmake/arm-compile6.cmake` - ARM Compiler 6
- `cmake/starm-clang.cmake` - STARM Clang

---

## Directory Structure

```
EmbeddedKit/
├── CMakeLists.txt              # Main build script
├── ek_conf.h                   # Global configuration file
├── CLAUDE.md                   # Project documentation and changelog
├── LICENSE                     # MIT License
├── cmake/                      # Toolchain files
│   ├── gcc-arm-none-eabi.cmake
│   ├── arm-compile6.cmake
│   └── starm-clang.cmake
├── L1_MCU/                     # Layer 1: MCU vendor libraries
│   ├── stub/                   # Weak definition stubs
│   └── STM32F429ZIT6_GCC/      # STM32F429 support
├── L2_Core/                    # Layer 2: Core & HAL
│   ├── utils/                  # Pure software utilities
│   ├── hal/                    # Hardware abstraction layer
│   └── third_party/            # Third-party libraries
├── L3_Middlewares/             # Layer 3: Middleware
│   ├── FreeRTOS/               # Real-time operating system
│   ├── FatFS/                  # File system
│   └── LVGL/                   # Graphics library
├── L4_Components/              # Layer 4: Device drivers (OOP)
└── L5_App/                     # Layer 5: Application logic
```

---

## Layers Description

### L1_MCU (MCU Vendor Library Layer)

Contains vendor-provided official libraries and startup code. Each MCU model has an independent subdirectory. The `main.c` is responsible for calling `ek_main()` after hardware initialization.

**Status**: STM32F429 support complete

### L2_Core (Core & Hardware Abstraction Layer)

**utils/ (Hardware-independent)**:
- `ek_def.h` - Common definitions and cross-compiler macros
- `ek_list.h` - Doubly linked list
- `ek_ringbuf.h` - Ring buffer with element counting
- `ek_stack.h` - Stack data structure
- `ek_vec.h` - Dynamic vector (array)
- `ek_mem.h` - TLSF-based dynamic memory management
- `ek_log.h` - Multi-level logging system
- `ek_assert.h` - Assertion module

**hal/ (Hardware Abstraction)**: Not yet implemented. Will provide logical-to-physical mapping (e.g., `HAL_GPIO_1` → `GPIOA PIN_5`).

**third_party/**:
- `lwprintf` - Lightweight formatted output library
- `tlsf` - Two-Level Segregated Fit memory allocator

### L3_Middlewares (Third-party Middleware Layer)

Each middleware has an independent subdirectory and CMakeLists.txt.

**Integrated Middleware**:

1. **FreeRTOS** - Real-time operating system
   - Port: GCC_ARM_CM4F (for STM32F429)
   - Heap implementation: heap_4.c
   - Configuration: 168MHz CPU, 1000Hz Tick Rate, 32KB heap
   - Enable via: `-DUSE_FREERTOS=ON`

2. **FatFS** - File system (framework ready)

3. **LVGL** - Lightweight graphics library (framework ready)

### L4_Components (Hardware Driver Component Layer)

**Core Design - OOP Pattern**:
- Interface abstraction using function pointers
- Object encapsulation with properties and methods
- Dependency inversion - components depend on abstract interfaces
- Polymorphism support - same component for multiple hardware types

**Status**: Directory created, components to be implemented

### L5_App (Application Layer)

Implements `ek_main()` as the application entry point. Should call L2~L4 layer services, not L1 directly.

**Current Implementation**:
```c
void ek_main(void)
{
    ek_heap_init();  // Initialize memory heap

    while (1)
    {
        // Business logic
    }
}
```

---

## Configuration

The global configuration file `ek_conf.h` in the root directory manages framework-wide settings:

```c
// RTOS Configuration
#define EK_USE_RTOS (1)

// Memory Management
#define EK_HEAP_NO_TLSF (0)
#define EK_HEAP_SIZE (30 * 1024)

// Module Enable/Disable
#define EK_IO_ENABLE (1)
#define EK_LOG_ENABLE (1)
#define EK_LIST_ENABLE (1)
#define EK_VEC_ENABLE (1)
#define EK_RINGBUF_ENABLE (1)
#define EK_STACK_ENABLE (1)

// Logging Configuration
#define EK_LOG_DEBUG_ENABLE (1)
#define EK_LOG_COLOR_ENABLE (1)
#define EK_LOG_BUFFER_SIZE (256)
```

---

## Project Status

### Completed
- ✅ Complete 5-layer architecture design
- ✅ CMake build system configuration
- ✅ L1_MCU: STM32F429 support complete (HAL library, startup files, main.c)
- ✅ L2_Core/utils: All utility modules implemented
- ✅ L2_Core/third_party: lwprintf and tlsf integrated
- ✅ L3_Middlewares: FreeRTOS fully integrated, FatFS and LVGL framework ready
- ✅ L5_App: Basic entry implementation
- ✅ Global configuration file `ek_conf.h`
- ✅ Detailed documentation

### To Be Implemented
- ⏳ L2_Core/hal: Hardware abstraction layer (GPIO, UART, I2C, SPI, etc.)
- ⏳ L4_Components: Specific device drivers (OLED, Flash, sensors, etc.)
- ⏳ L5_App: Specific business logic applications

---

## Contributing

Contributions are welcome! Please follow the architectural rules and coding standards described in this document.

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
