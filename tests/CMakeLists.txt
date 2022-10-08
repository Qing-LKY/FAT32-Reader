# If pkg-config is not installed on the system, then the
# CHECK_INSTALL_DIR variable must be set to the install
# location of Check. For example, on Windows, this may
# be: C:/Program Files/check
# set(CHECK_INSTALL_DIR "C:/Program Files/check")

find_package(Check REQUIRED)

include(CheckCSourceCompiles)
include(CheckCSourceRuns)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckSymbolExists)
include(CheckTypeSize)


add_executable(fat_test fat_test.c)
include_directories(${PROJECT_SOURCE_DIR}/include)
target_link_libraries(fat_test fat32-lib ${CHECK_LIBRARIES} pthread)

file(COPY a.img DESTINATION .)

add_test(NAME fat_test COMMAND fat_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(fat_test PROPERTIES TIMEOUT 30)
