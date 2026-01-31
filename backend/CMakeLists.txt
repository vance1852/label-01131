cmake_minimum_required(VERSION 3.16)
project(process_manager VERSION 1.0.0 LANGUAGES CXX)

# C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()

# 源文件
set(SOURCES
    src/main.cpp
    src/process_manager.cpp
    src/terminal.cpp
)

# 头文件目录
include_directories(src)

# 可执行文件
add_executable(process_manager ${SOURCES})

# 安装规则
install(TARGETS process_manager DESTINATION bin)
