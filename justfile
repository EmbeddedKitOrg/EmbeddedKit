alias b := build
alias bg := build-gd
alias c := clean
alias t := test

build:
    @cmake -B build -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE="cmake/gcc-arm-none-eabi.cmake" \
      -DCMAKE_BUILD_TYPE=Debug \
      -DMCU_MODEL="STM32F429ZIT6_GCC" \
      -DUSE_FREERTOS=OFF \
      -DUSE_FATFS=OFF \
      -DUSE_LVGL=OFF
    @ninja -C build

build-gd:
    @cmake -B build -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE="cmake/gcc-arm-none-eabi.cmake" \
        -DCMAKE_BUILD_TYPE=Debug \
        -DMCU_MODEL="GD32F470ZGT6" \
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