include_directories(${PROJECT_SOURCE_DIR}/include)

set(SOURCE
    fat.c
    io.c
    main.c
    interact.c
)
add_executable(
    fat32-reader
    ${SOURCE}
)

target_link_libraries(fat32-reader readline)
add_library(
    fat32-lib
    ${SOURCE}
)
