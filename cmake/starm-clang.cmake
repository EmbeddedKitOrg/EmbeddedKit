# =============================================================================
# 基础系统配置
# =============================================================================
set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          arm)

# 注意：ST 的工具链本质是 LLVM/Clang，所以 ID 设为 Clang
set(CMAKE_C_COMPILER_ID             Clang)
set(CMAKE_CXX_COMPILER_ID           Clang)

# =============================================================================
# 工具链路径配置
# =============================================================================
# [需修改] 确保 'starm-' 在你的环境变量 PATH 中，或者写绝对路径
# 例如 C:/ST/STM32CubeCLT/STLink-gdb-server/bin/starm-
set(TOOLCHAIN_PREFIX                starm-)

set(CMAKE_C_COMPILER                ${TOOLCHAIN_PREFIX}clang)
set(CMAKE_ASM_COMPILER              ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER              ${TOOLCHAIN_PREFIX}clang++)
set(CMAKE_LINKER                    ${TOOLCHAIN_PREFIX}clang)
set(CMAKE_OBJCOPY                   ${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_SIZE                      ${TOOLCHAIN_PREFIX}size)

set(CMAKE_EXECUTABLE_SUFFIX_ASM     ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_C       ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX     ".elf")

# 跳过编译检查，防止配置阶段报错
set(CMAKE_TRY_COMPILE_TARGET_TYPE   STATIC_LIBRARY)

# =============================================================================
# C 库模式选择 (ST工具链特有)
# =============================================================================
# [可修改] 选择使用的 C 库类型：
# "STARM_PICOLIBC" : (推荐) 专为嵌入式优化的现代化 C 库，体积小，速度快
# "STARM_NEWLIB"   : 传统的嵌入式 C 库
# "STARM_HYBRID"   : 混合模式 (Clang 编译器 + GNU 库)
set(STARM_TOOLCHAIN_CONFIG "STARM_PICOLIBC")

# 根据选择配置库路径 (通常不需要改)
if(STARM_TOOLCHAIN_CONFIG STREQUAL "STARM_HYBRID")
  set(TOOLCHAIN_MULTILIBS "--multi-lib-config=\"$ENV{CLANG_GCC_CMSIS_COMPILER}/multilib.gnu_tools_for_stm32.yaml\" --gcc-toolchain=\"$ENV{GCC_TOOLCHAIN_ROOT}/..\"")
elseif (STARM_TOOLCHAIN_CONFIG STREQUAL "STARM_NEWLIB")
  set(TOOLCHAIN_MULTILIBS "--config=newlib.cfg")
endif()

# =============================================================================
# 芯片架构参数 (针对 STM32F429)
# =============================================================================
# [需修改] F429 是 Cortex-M4F 核心
# -mcpu=cortex-m4       : 核心类型
# -mfpu=fpv4-sp-d16     : F429 的 FPU 版本 (单精度)
# -mfloat-abi=hard      : 开启硬件浮点
set(TARGET_FLAGS "-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard ${TOOLCHAIN_MULTILIBS}")

# =============================================================================
# 编译器选项 (Compiler Flags)
# =============================================================================
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TARGET_FLAGS}")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp -MP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fdata-sections -ffunction-sections")

# 调试/发布预设
# -Og: 调试友好优化
# -Oz: 极致体积优化 (LLVM 特有)
set(CMAKE_C_FLAGS_DEBUG "-Og -g3")
set(CMAKE_C_FLAGS_RELEASE "-Oz -g0")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-Oz -g0")

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fno-rtti -fno-exceptions -fno-threadsafe-statics")

# =============================================================================
# 链接脚本配置
# =============================================================================
#  定义变量 LINKER_SCRIPT
#    CACHE FILEPATH: 表示这是一个缓存变量，类型为文件路径，可以在命令行被 -D 覆盖
#    默认值: 设置为你原本的路径
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/L1_MCU/STM32F429VGT6/STM32F429XX_FLASH.ld"
    CACHE FILEPATH "The path to the linker script")

if(NOT CMAKE_SOURCE_DIR MATCHES "CMakeScratch")
    if(NOT EXISTS "${LINKER_SCRIPT}")
        message(WARNING "Linker script not found at: ${LINKER_SCRIPT}")
    endif()
endif()


# =============================================================================
#  链接器选项 (Linker Flags)
# =============================================================================
set(CMAKE_EXE_LINKER_FLAGS "${TARGET_FLAGS}")

# 库启动文件配置
if (STARM_TOOLCHAIN_CONFIG STREQUAL "STARM_HYBRID")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --gcc-specs=nano.specs")
  set(TOOLCHAIN_LINK_LIBRARIES "m")
elseif(STARM_TOOLCHAIN_CONFIG STREQUAL "STARM_NEWLIB")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lcrt0-nosys")
elseif(STARM_TOOLCHAIN_CONFIG STREQUAL "STARM_PICOLIBC")
  # Picolibc 启动配置
  # -lcrt0-hosted : 包含标准启动代码 (替代 startup.s 中的部分功能)
  # -z norelro    : 禁用只读重定位，节省 RAM
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lcrt0-hosted -z norelro")
endif()

# [需修改] 指定链接脚本 (.ld)
# 请确保该文件路径正确，通常在 L1_MCU 目录下
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T \"${LINKER_SCRIPT}\"")

# 生成 Map 文件、裁剪未使用的段、打印内存占用
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${CMAKE_PROJECT_NAME}.map -Wl,--gc-sections")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -z noexecstack")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--print-memory-usage")
