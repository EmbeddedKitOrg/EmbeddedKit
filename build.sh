cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE="cmake/gcc-arm-none-eabi.cmake" -DLINKER_SCRIPT="L1_MCU/STM32F429VGT6/STM32F429XX_FLASH.ld"
ninja -C build
