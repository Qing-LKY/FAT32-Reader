project(fat32-reader)

cmake_minimum_required(VERSION 2.9)

include_directories("include")

add_executable(
    fat32-reader
    src/io.c
    src/main.c
)