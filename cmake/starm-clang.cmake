set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          arm)
set(CMAKE_C_COMPILER_ID             Clang)
set(CMAKE_CXX_COMPILER_ID           Clang)

# 工具链前缀，确保在 PATH 中或使用绝对路径
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
set(CMAKE_TRY_COMPILE_TARGET_TYPE   STATIC_LIBRARY)

# C 库选择: STARM_PICOLIBC / STARM_NEWLIB / STARM_HYBRID
set(STARM_TOOLCHAIN_CONFIG "STARM_PICOLIBC")

if(STARM_TOOLCHAIN_CONFIG STREQUAL "STARM_HYBRID")
  set(TOOLCHAIN_MULTILIBS "--multi-lib-config=\"$ENV{CLANG_GCC_CMSIS_COMPILER}/multilib.gnu_tools_for_stm32.yaml\" --gcc-toolchain=\"$ENV{GCC_TOOLCHAIN_ROOT}/..\"")
elseif (STARM_TOOLCHAIN_CONFIG STREQUAL "STARM_NEWLIB")
  set(TOOLCHAIN_MULTILIBS "--config=newlib.cfg")
endif()

# 芯片架构: Cortex-M4 with FPU
set(TARGET_FLAGS "-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard ${TOOLCHAIN_MULTILIBS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TARGET_FLAGS}")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp -MP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fdata-sections -ffunction-sections")

set(CMAKE_C_FLAGS_DEBUG "-Og -g3")
set(CMAKE_C_FLAGS_RELEASE "-Oz -g0")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-Oz -g0")

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fno-rtti -fno-exceptions -fno-threadsafe-statics")

# 链接脚本路径 (可通过 -DLINKER_SCRIPT 覆盖)
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/L1_MCU/STM32F429ZIT6/stm32f429zit6_flash.ld"
    CACHE FILEPATH "The path to the linker script")

if(NOT CMAKE_SOURCE_DIR MATCHES "CMakeScratch")
    if(NOT EXISTS "${LINKER_SCRIPT}")
        message(WARNING "Linker script not found at: ${LINKER_SCRIPT}")
    endif()
endif()

set(CMAKE_EXE_LINKER_FLAGS "${TARGET_FLAGS}")
if (STARM_TOOLCHAIN_CONFIG STREQUAL "STARM_HYBRID")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --gcc-specs=nano.specs")
  set(TOOLCHAIN_LINK_LIBRARIES "m")
elseif(STARM_TOOLCHAIN_CONFIG STREQUAL "STARM_NEWLIB")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lcrt0-nosys")
elseif(STARM_TOOLCHAIN_CONFIG STREQUAL "STARM_PICOLIBC")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lcrt0-hosted -z norelro")
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T \"${LINKER_SCRIPT}\"")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${CMAKE_PROJECT_NAME}.map -Wl,--gc-sections")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -z noexecstack")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--print-memory-usage")
