include_directories(${PROJECT_SOURCE_DIR}/include)

set(SOURCE
    fat.c
    io.c
    main.c
)
add_executable(
    fat32-reader
    ${SOURCE}
)
add_library(
    fat32-lib
    ${SOURCE}
)
