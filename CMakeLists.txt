cmake_minimum_required(VERSION 3.20)
project(pg_hexretro C)

set(CMAKE_C_STANDARD 11)

include_directories(.)
aux_source_directory(. SRC_LIST)

add_executable(pg_hexretro ${SRC_LIST})
