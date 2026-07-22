cmake_minimum_required(VERSION 3.16)

project(RedisLikeKeyValueStore VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

include_directories(include)

add_executable(redis_server
    src/main.cpp
    src/key_value_store.cpp
    src/command_parser.cpp
)
