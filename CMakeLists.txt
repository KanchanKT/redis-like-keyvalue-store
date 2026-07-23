cmake_minimum_required(VERSION 3.16)

project(RedisLikeKeyValueStore VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

include_directories(include)

enable_testing()

add_executable(redis_server
    src/main.cpp
    src/key_value_store.cpp
    src/command_parser.cpp
    src/wal.cpp
)

add_executable(test_key_value_store
    tests/test_key_value_store.cpp
    src/key_value_store.cpp
    src/command_parser.cpp
    src/wal.cpp
)

target_include_directories(test_key_value_store PRIVATE include)

add_test(NAME key_value_store_tests COMMAND test_key_value_store)
