# 添加cubemx库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cmake/stm32cubemx)

# 添加源码 (Core/Src + Weak Main)
target_sources(l1_mcu PRIVATE
    ${USER_SRCS}
    ${GEN_WEAK_MAIN_SRC}
)

#  链接 CubeMX 库
#    这里使用 PUBLIC，这样 l1_mcu 就会自动继承 HAL 库的所有头文件路径和定义
target_link_libraries(l1_mcu PUBLIC
    stm32cubemx
)

# 链接全局配置 (确保 CubeMX 代码也能吃到你的全局编译参数)
#    注意：有时候 CubeMX 自带的配置和 Global 冲突，如果报错可以去掉这一行
#    注意：stm32cubemx 是 INTERFACE 库，只能使用 INTERFACE 关键字
target_link_libraries(stm32cubemx INTERFACE
    global_macros
    global_options
)