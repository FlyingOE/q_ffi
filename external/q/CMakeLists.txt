# vim: set et noai ts=4 sw=4 syntax=cmake:
cmake_minimum_required(VERSION 3.15)

include(CMakePrintHelpers)

########################################
project(External_kdb.q)
set(VERSION 3.0)
set(DESCRIPTION "kdb+ 3.x C API")

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
elseif(UNIX AND NOT APPLE)
    set(k4_SYSTEM "l")
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86")
        set(k4_PLATFORM "l32")
    elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        set(k4_PLATFORM "l64")
    else()
        message(FATAL_ERROR "Failed to detect Linux platform: CMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}")
    endif()
elseif(APPLE)
    set(k4_SYSTEM "m")
    message(FATAL_ERROR "FIXME: macOS not support yet!")
else()
    message(FATAL_ERROR "FIXME: Unsupport target platform!")
endif()

# Library contents
cmake_print_variables(k4_SYSTEM)
cmake_print_variables(k4_PLATFORM)

set(target_name kdb_C_API)
if(k4_SYSTEM STREQUAL "w")
    add_library(${target_name} SHARED IMPORTED GLOBAL)
    target_include_directories(${target_name}
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_target_properties(${target_name} PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/${k4_PLATFORM}/c.dll
        IMPORTED_IMPLIB ${CMAKE_CURRENT_SOURCE_DIR}/${k4_PLATFORM}/c.lib
    )
elseif(k4_SYSTEM STREQUAL "l")
    add_library(${target_name} OBJECT IMPORTED GLOBAL)
    target_include_directories(${target_name}
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_target_properties(${target_name} PROPERTIES
        #IMPORTED_OBJECTS ${CMAKE_CURRENT_SOURCE_DIR}/${k4_PLATFORM}/c.o
        IMPORTED_OBJECTS ${CMAKE_CURRENT_SOURCE_DIR}/${k4_PLATFORM}/e.o
    )
elseif(k4_SYSTEM STREQUAL "m")
    # FIXME
else()
    # FIXME
endif()

# Alias for export
add_library(kdb::C ALIAS ${target_name})

cmake_print_properties(
    TARGETS kdb::C
    PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES
        IMPORTED_LOCATION
        IMPORTED_IMPLIB
        IMPORTED_OBJECTS
)
