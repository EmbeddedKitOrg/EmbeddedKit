# 路径中需要有 `starm-clang`(ST官方的STM32CubeCLT中可以下载)
cmake -B build -G Ninja\
      -DCMAKE_TOOLCHAIN_FILE="cmake/starm-clang.cmake"\
      -DCMAKE_BUILD_TYPE=Debug\
      -DLINKER_SCRIPT="L1_MCU/STM32F429ZIT6_STARM/stm32f429zit6_flash.ld" \
      -DMCU_MODEL="STM32F429ZIT6_STARM"\
      -DUSE_FREERTOS=OFF\
      -DUSE_FATFS=OFF\
      -DUSE_LVGL=OFF\

ninja -C build
