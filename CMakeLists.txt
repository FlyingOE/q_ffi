### vim: set et noai ts=4 sw=4 syntax=cmake:
cmake_minimum_required(VERSION 3.12)
set(CMAKE_VERBOSE_MAKEFILE ON)
if(POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)    # project(VERSION)
endif()
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)    # set(CMAKE_MSVC_RUNTIME_LIBRARY)
endif()
include(CMakePrintHelpers)
include(ExternalProject)
find_package(Git REQUIRED)

########################################
project(q_ffi
    VERSION 0.1
    DESCRIPTION "A k9-like FFI for kdb+/q"
    HOMEPAGE_URL https://github.com/FlyingOE/q_ffi
)

# Build requirements
enable_language(CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FAIL_ERROR "In-source builds are not allowed.")
endif ()

# Project global setup
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(q_ffi_DEPENDENCIES   ${PROJECT_SOURCE_DIR}/depends)
set(q_ffi_DISTRIBUTABLES ${PROJECT_SOURCE_DIR}/dist)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    message(STATUS "Setting build type to `${CMAKE_BUILD_TYPE}' as none was specified.")
    # Possible build types for CMake GUI
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
    )
endif()

# Platform detection
if(WIN32)
    if(NOT (MSVC OR MSYS OR MINGW))
        message(FATAL_ERROR "FIXME: Support non-MSVC?")
    endif()

    set(k4_SYSTEM "w")
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(k4_PLATFORM "w32")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(k4_PLATFORM "w64")
    else()
        message(FATAL_ERROR "Unknown sizeof(void*): CMAKE_SIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P}")
    endif()

    add_compile_options(/nologo)
    add_compile_options(/MP)        # multi-threaded build
    add_compile_options(/W4 /WX)    # warning level
    string(FIND "${CMAKE_CXX_FLAGS}" "/W3" NEED_FIX_W3)
    if(NOT NEED_FIX_W3 EQUAL -1)    # work-around conflicting /W3 and /W4
        string(REPLACE "/W3" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endif()
    remove_definitions(_MBCS)
    add_compile_definitions(_UNICODE)
    add_link_options(/nologo)

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

# Embed Git hash into binary
execute_process(
	COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
	OUTPUT_VARIABLE q_ffi_GIT_HASH
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Project contents
configure_file(q_ffi_config.h.in q_ffi.h @ONLY)
include_directories(${PROJECT_BINARY_DIR})

add_subdirectory(q_ffi)

# Unit testing
enable_testing()
add_subdirectory(test)
