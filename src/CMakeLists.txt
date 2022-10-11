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

install(TARGETS fat32-reader EXPORT fat32-reader-debug
        CONFIGURATIONS Debug
        RUNTIME
        DESTINATION ${PROJECT_BINARY_DIR}/bin)

install(TARGETS fat32-reader
        CONFIGURATIONS Release
        RUNTIME
        DESTINATION ${PROJECT_BINARY_DIR}/bin)

add_library(
    fat32-lib
    ${SOURCE}
)
