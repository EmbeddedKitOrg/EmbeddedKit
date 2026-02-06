# 路径中需要有 `gcc-arm-none-eabi`
cmake -B build -G Ninja\
      -DCMAKE_TOOLCHAIN_FILE="cmake/gcc-arm-none-eabi.cmake"\
      -DCMAKE_BUILD_TYPE=Debug\
      -DLINKER_SCRIPT="L1_MCU/STM32F407VGT6_GCC/stm32f407vgt6_flash.ld" \
      -DMCU_MODEL="STM32F407VGT6_GCC"\
      -DUSE_FREERTOS=OFF\
      -DUSE_FATFS=OFF\
      -DUSE_LVGL=OFF

ninja -C build
