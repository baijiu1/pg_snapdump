cmake_minimum_required(VERSION 3.23)
project(pg_snapdump)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -g")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_compile_definitions(LOG_LEVEL=1)
#option(LOG_LEVEL "program putput log level control" 0)
#if (LOG_LEVEL)
#    add_definitions(-DLOG_LEVEL)
#else ()
#
#endif ()

# 可选：确保使用 C++17 编译
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 例如，设置全局的构建类型
set(CMAKE_BUILD_TYPE Release)

include_directories(.)
aux_source_directory(. SRC_LIST)

add_executable(pg_snpdump ${SRC_LIST})