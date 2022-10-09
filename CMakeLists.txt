project(fat32-reader)

cmake_minimum_required(VERSION 2.9)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set_property(GLOBAL PROPERTY C_STANDARD 11)

add_subdirectory(src)
add_subdirectory(tests)

enable_testing()
add_test(NAME fat_test COMMAND fat_test)
