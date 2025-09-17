cmake_minimum_required(VERSION 3.23)
project(pg_snapdump)

set(CMAKE_CXX_STANDARD 14)
add_compile_options(-g)      # 生成调试信息
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -g")
add_compile_definitions(LOG_LEVEL=1)


#find_package(PostgreSQL REQUIRED)

# 可选：确保使用 C++17 编译
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 例如，设置全局的构建类型
set(CMAKE_BUILD_TYPE Release)

include_directories(.)
aux_source_directory(. SRC_LIST)

add_executable(pg_snpdump ${SRC_LIST})
#target_link_libraries(pg_snpdump PRIVATE PostgreSQL::PostgreSQL)
