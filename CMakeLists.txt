### vim: set et noai ts=4 sw=4 syntax=cmake:
cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0048 NEW)    # project(VERSION)
cmake_policy(SET CMP0091 NEW)    # set(CMAKE_MSVC_RUNTIME_LIBRARY)
include(CMakePrintHelpers)

########################################
project(q_ffi
    VERSION 0.1
    DESCRIPTION "A k9-like FFI for kdb+/q"
    HOMEPAGE_URL "https://github.com/FlyingOE/q_ffi")

# Build requirements
enable_language(CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Project global setup
set_property(GLOBAL PROPERTY USE_FOLDERS On)
set(q_ffi_DEPENDENCIES   ${PROJECT_SOURCE_DIR}/dependencies)
set(q_ffi_DISTRIBUTABLES ${PROJECT_SOURCE_DIR}/dist)

# Platform detection
if(WIN32)
    if(NOT (MSVC OR MSYS OR MINGW))
        message(FATAL_ERROR "FIXME: Support for non-MSVC needed...")
    endif()

    set(k4_SYSTEM "w")
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(k4_PLATFORM "w32")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(k4_PLATFORM "w64")
    else()
        message(FATAL_ERROR "Unknown sizeof(void*): CMAKE_SIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P}")
    endif()

    remove_definitions(_MBCS)
    add_compile_definitions(_UNICODE)
    add_compile_options(/W4 /WX)
    string(REPLACE "/W3" ""     # work-around conflicting /W3 and /W4
        CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    add_compile_options(/MP)    # multi-threaded build

elseif(UNIX AND NOT APPLE)
    set(k4_SYSTEM "l")
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86")
        set(k4_PLATFORM "l32")
    elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        set(k4_PLATFORM "l64")
    else()
        message(FATAL_ERROR "Failed to detect Linux platform: CMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}")
    endif()
    
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)

elseif(APPLE)
    set(k4_SYSTEM "m")
    message(FATAL_ERROR "FIXME: macOS not support yet!")
else()
    message(FATAL_ERROR "FIXME: Unsupport target platform!")
endif()

cmake_print_variables(k4_SYSTEM)
cmake_print_variables(k4_PLATFORM)

# Project contents
configure_file(q_ffi_config.h.in q_ffi.h @ONLY)
include_directories(${PROJECT_BINARY_DIR})

add_subdirectory(q_ffi)
