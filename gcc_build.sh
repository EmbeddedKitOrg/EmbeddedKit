# 路径中需要有 `gcc-arm-none-eabi`
cmake -B build -G Ninja\
      -DCMAKE_TOOLCHAIN_FILE="cmake/gcc-arm-none-eabi.cmake"\
      -DCMAKE_BUILD_TYPE=Debug\
      -DLINKER_SCRIPT="L1_MCU/STM32F429ZIT6_GCC/STM32F429XX_FLASH.ld" \
      -DMCU_MODEL="STM32F429ZIT6_GCC"\
      -DUSE_FREERTOS=OFF\
      -DUSE_FATFS=OFF\
      -DUSE_LVGL=OFF\
      -DUSE_LVGL_THORVG=OFF

ninja -C build
