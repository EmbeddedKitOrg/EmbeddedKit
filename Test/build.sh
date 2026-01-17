set -e
cmake -DCMAKE_C_FLAGS="-m32"\
   -DCMAKE_CXX_FLAGS="-m32"\
   -B build \
   -G Ninja \
   -DCMAKE_BUILD_TYPE=Debug

   
ninja -C build
./build/test
