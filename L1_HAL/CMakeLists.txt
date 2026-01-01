# 收集源文件
file(GLOB L1_SRCS "src/*.c")

# 定义库
add_library(l1_hal STATIC ${L1_SRCS})

# 设置头文件路径
target_include_directories(l1_hal PUBLIC inc)

# 设置依赖
target_link_libraries(l1_hal PUBLIC 
    l0_mcu          # 依赖 L0 (为了访问寄存器定义等)
    global_macros   # 继承全局宏
    global_options  # 继承全局编译选项
)
