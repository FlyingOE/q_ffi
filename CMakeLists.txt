# vim: set et noai ts=4 sw=4 syntax=cmake:
cmake_minimum_required(VERSION 3.15)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    message(STATUS "Setting build type to `${CMAKE_BUILD_TYPE}' as none was specified.")
    # Possible build types for CMake GUI
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
    )
endif()

####################
project(q_ffi
    VERSION      0.1.0
    DESCRIPTION  "A k9-like FFI for kdb+/q"
    HOMEPAGE_URL "https://github.com/FlyingOE/q_ffi"
    LANGUAGES    CXX
)

# Global environment
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE   ON)
include(CTest)

# External dependencies
add_subdirectory(external)

# Main DLL & library
add_subdirectory(q_ffi)

# Unit tests
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
