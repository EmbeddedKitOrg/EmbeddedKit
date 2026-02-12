alias b := build
alias bg := build-gd
alias bs := build-starm
alias c := clean
alias t := test

build:
    @cmake -B build -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE="cmake/gcc-arm-none-eabi.cmake" \
      -DCMAKE_BUILD_TYPE=Debug \
      -DLINKER_SCRIPT="L1_MCU/STM32F429ZIT6_GCC/stm32f429zit6_flash.ld"  \
      -DMCU_MODEL="STM32F429ZIT6_GCC" \
      -DUSE_FREERTOS=OFF \
      -DUSE_FATFS=OFF \
      -DUSE_LVGL=OFF
    @ninja -C build

build-gd:
    @cmake -B build -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE="cmake/gcc-arm-none-eabi.cmake" \
        -DCMAKE_BUILD_TYPE=Debug \
        -DLINKER_SCRIPT="L1_MCU/GD32F470ZGT6/gd32f470zgt6_flash.ld"  \
        -DMCU_MODEL="GD32F470ZGT6" \
        -DUSE_FREERTOS=OFF \
        -DUSE_FATFS=OFF \
        -DUSE_LVGL=OFF
    @ninja -C build

build-starm:
    @cmake -B build -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE="cmake/starm-clang.cmake" \
      -DCMAKE_BUILD_TYPE=Debug \
      -DLINKER_SCRIPT="L1_MCU/STM32F429ZIT6_STARM/stm32f429zit6_flash.ld"  \
      -DMCU_MODEL="STM32F429ZIT6_STARM" \
      -DUSE_FREERTOS=OFF \
      -DUSE_FATFS=OFF \
      -DUSE_LVGL=OFF
    @ninja -C build

clean:
    @rm -rf build
    @echo 'build directory has been removed'

[working-directory: './Test']
test:
    @cmake -DCMAKE_C_FLAGS="-m32" \
        -DCMAKE_CXX_FLAGS="-m32" \
        -B build \
        -G Ninja \
        -DCMAKE_BUILD_TYPE=Debug

    @ninja -C build
    @./build/test