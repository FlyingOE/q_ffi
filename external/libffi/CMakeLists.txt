# vim: set et noai ts=4 sw=4 syntax=cmake:
cmake_minimum_required(VERSION 3.5)

set(LIBFFI_NAME    libffi)
set(LIBFFI_VERSION 3.3)

# @ref https://github.com/HowardHinnant/date/blob/master/CMakeLists.txt
# @ref https://cmake.org/cmake/help/git-stage/module/FetchContent.html
include(FetchContent)

# FIXME: Main branch has not yet merged with CMake support!
FetchContent_Declare(${LIBFFI_NAME}
#    GIT_REPOSITORY https://github.com/libffi/libffi.git
#    GIT_TAG        v${LIBFFI_VERSION}
    GIT_REPOSITORY https://github.com/am11/libffi/  # Add CMake support
    GIT_TAG        feature/cmake-build-configs
)

message(STATUS "GitHub >> Fetching ${LIBFFI_NAME}/v${LIBFFI_VERSION} ...")

set(FFI_CLOSURES 1 CACHE STRING "" FORCE)
set(FFI_DEBUG 0 CACHE STRING "" FORCE)
#set(FFI_NO_STRUCTS 0 CACHE STRING "" FORCE)
#set(FFI_NO_RAW_API 0 CACHE STRING "" FORCE)
#set(FFI_EXEC_TRAMPOLINE_TABLE 0 CACHE STRING "" FORCE)

FetchContent_MakeAvailable(${LIBFFI_NAME})

message(STATUS "------ << Updated ${LIBFFI_NAME}/v${LIBFFI_VERSION} .")

#========================================

# Differentiate shared and static builds & provide corresponding aliases
set_target_properties(ffi_shared PROPERTIES
    OUTPUT_NAME libffi-${LIBFFI_VERSION}
)
add_library(libffi::shared ALIAS ffi_shared)

set_target_properties(ffi_static PROPERTIES
    OUTPUT_NAME libffi
)
add_library(libffi::static ALIAS ffi_static)

# Fix header files due to libffi using global CMAKE_BINARY_DIR (old CMake 2.8.x)
get_target_property(bin_dir ffi_shared BINARY_DIR)
foreach(header
        ffi.h fficonfig.h ffitarget.h)
    configure_file(${CMAKE_BINARY_DIR}/include/${header} ${bin_dir}/include/${header} COPYONLY)
endforeach()

# ---
cmake_print_properties(
    TARGETS libffi::shared
    PROPERTIES
        SOURCE_DIR
        BINARY_DIR
        INCLUDE_DIRECTORIES
)
cmake_print_properties(
    TARGETS libffi::static
    PROPERTIES
        SOURCE_DIR
        BINARY_DIR
        INCLUDE_DIRECTORIES
)
