# =============================================================================
# 基础系统与编译器识别
# =============================================================================
set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          arm)

# [注意] 这里使用的是 "Clang"，不是 "ARMClang"
# 这意味着 CMake 会把它当做标准 LLVM 处理，支持 GCC 风格的参数
set(CMAKE_C_COMPILER_ID             Clang)
set(CMAKE_CXX_COMPILER_ID           Clang)

# =============================================================================
# 工具链路径配置
# =============================================================================
# [需修改] ST 的 LLVM 工具链前缀是 "starm-"
# 确保你的环境变量里已经有 STM32CubeCLT 的 bin 目录
set(TOOLCHAIN_PREFIX                starm-)

set(CMAKE_C_COMPILER                ${TOOLCHAIN_PREFIX}clang)
set(CMAKE_ASM_COMPILER              ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER              ${TOOLCHAIN_PREFIX}clang++)
set(CMAKE_LINKER                    ${TOOLCHAIN_PREFIX}clang)    # 链接器也用 clang 前端调用
set(CMAKE_OBJCOPY                   ${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_SIZE                      ${TOOLCHAIN_PREFIX}size)

set(CMAKE_EXECUTABLE_SUFFIX_ASM     ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_C       ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX     ".elf")

# 同样，跳过可执行文件检查，防止配置阶段报错
set(CMAKE_TRY_COMPILE_TARGET_TYPE   STATIC_LIBRARY)

# =============================================================================
# 核心配置开关 (ST 工具链特有功能)
# =============================================================================
# ST 提供了三种库模式，你可以根据需要修改这个变量：
# "STARM_HYBRID"   : 混合模式 (Clang 编译器 + GNU 链接器/库)
# "STARM_NEWLIB"   : 纯 Clang + Newlib 库 (标准嵌入式库)
# "STARM_PICOLIBC" : 纯 Clang + Picolibc (更小、更快的现代化库，强烈推荐)
set(STARM_TOOLCHAIN_CONFIG "STARM_PICOLIBC")

# 根据上面的选择，自动设置库的路径和配置文件
if(STARM_TOOLCHAIN_CONFIG STREQUAL "STARM_HYBRID")
  # 混合模式需要指定 GNU 工具链的位置
  set(TOOLCHAIN_MULTILIBS "--multi-lib-config=\"$ENV{CLANG_GCC_CMSIS_COMPILER}/multilib.gnu_tools_for_stm32.yaml\" --gcc-toolchain=\"$ENV{GCC_TOOLCHAIN_ROOT}/..\"")
elseif (STARM_TOOLCHAIN_CONFIG STREQUAL "STARM_NEWLIB")
  set(TOOLCHAIN_MULTILIBS "--config=newlib.cfg")
endif()

# =============================================================================
# 芯片架构参数 (针对 STM32U575 - Cortex-M33)
# =============================================================================
# [需修改] 这里配置的是 Cortex-M33
set(TARGET_FLAGS "-mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard ${TOOLCHAIN_MULTILIBS}")

# =============================================================================
# 编译器选项 (Compiler Flags)
# =============================================================================
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TARGET_FLAGS}")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp -MP")

# -Wall: 开启警告
# -fdata-sections -ffunction-sections: 配合链接器的 gc-sections 裁剪体积
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fdata-sections -ffunction-sections")

# 调试/发布模式预设
# -Og: 针对调试优化的等级 (比 -O0 稍微优化一点，但能看清变量)
# -Oz: LLVM 特有的"超级体积优化" (比 GCC 的 -Os 更激进)
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
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/L0_MCU/STM32F429VGT6/STM32F429XX_FLASH.ld" 
    CACHE FILEPATH "The path to the linker script")

if(NOT CMAKE_SOURCE_DIR MATCHES "CMakeScratch")
    if(NOT EXISTS "${LINKER_SCRIPT}")
        message(WARNING "Linker script not found at: ${LINKER_SCRIPT}")
    endif()
endif()

# =============================================================================
# 链接器选项 (Linker Flags)
# =============================================================================
set(CMAKE_EXE_LINKER_FLAGS "${TARGET_FLAGS}")

# 根据所选的 C 库模式，链接不同的启动文件 (Startup Files)
if (STARM_TOOLCHAIN_CONFIG STREQUAL "STARM_HYBRID")
  # 混合模式使用 GCC 的 nano.specs
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --gcc-specs=nano.specs")
  set(TOOLCHAIN_LINK_LIBRARIES "m")

elseif(STARM_TOOLCHAIN_CONFIG STREQUAL "STARM_NEWLIB")
  # 链接 Newlib 的启动代码
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lcrt0-nosys")

elseif(STARM_TOOLCHAIN_CONFIG STREQUAL "STARM_PICOLIBC")
  # 链接 Picolibc 的启动代码
  # -lcrt0-hosted: 包含基本的 CRT0 启动代码
  # -z norelro: 禁用 Relocation Read-Only (为了节省 RAM)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lcrt0-hosted -z norelro")
endif()

# 因为 starm-clang 实际上是调用 ld.lld 或 arm-ld，所以使用 GCC 风格的 -T
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T \"${LINKER_SCRIPT}\"")

# 生成 Map 文件
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${CMAKE_PROJECT_NAME}.map")
# 裁剪未使用的段 (Dead Code Elimination)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
# 禁止栈执行 (安全特性)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -z noexecstack")
# 打印内存占用
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--print-memory-usage ")
