# vim: set et noai ts=4 sw=4 syntax=cmake:
cmake_minimum_required(VERSION 3.15)

include(CMakePrintHelpers)

########################################
project(External_kdb.q
    VERSION 3.0
    DESCRIPTION "kdb+ 3.x C API"
    HOMEPAGE_URL "https://github.com/KxSystems/kdb"
)

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
if(k4_SYSTEM STREQUAL "w")
    # A DLL callable from q
    add_library(kdb_C_API_DLL INTERFACE IMPORTED GLOBAL)
    target_include_directories(kdb_C_API_DLL
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_link_libraries(kdb_C_API_DLL
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/${k4_PLATFORM}/q.lib
    )

    # A stand-alone EXE
    add_library(kdb_C_API_EXE SHARED IMPORTED GLOBAL)
    target_include_directories(kdb_C_API_EXE
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_target_properties(kdb_C_API_EXE PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/${k4_PLATFORM}/c.dll
        IMPORTED_IMPLIB ${CMAKE_CURRENT_SOURCE_DIR}/${k4_PLATFORM}/c.lib
    )

elseif(k4_SYSTEM STREQUAL "l")
    # An SO callable from q
    add_library(kdb_C_API_DLL OBJECT IMPORTED GLOBAL)
    target_include_directories(kdb_C_API_DLL
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_target_properties(kdb_C_API_EXE PROPERTIES
        IMPORTED_OBJECTS ${CMAKE_CURRENT_SOURCE_DIR}/${k4_PLATFORM}/c.o
    )

    # An stand-alone EXE
    add_library(kdb_C_API_EXE OBJECT IMPORTED GLOBAL)
    target_include_directories(kdb_C_API_EXE
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_target_properties(kdb_C_API_EXE PROPERTIES
        IMPORTED_OBJECTS ${CMAKE_CURRENT_SOURCE_DIR}/${k4_PLATFORM}/e.o
    )

elseif(k4_SYSTEM STREQUAL "m")
    # FIXME
else()
    # FIXME
endif()

# Alias for export
add_library(kdb::C-dll ALIAS kdb_C_API_DLL)
add_library(kdb::C-exe ALIAS kdb_C_API_EXE)

set(k4_SYSTEM "${k4_SYSTEM}" CACHE INTERNAL "w|l|m")
set(k4_PLATFORM "${k4_PLATFORM}" CACHE INTERNAL "(w|l|m)(32|64)")
cmake_print_variables(k4_SYSTEM k4_PLATFORM)
#cmake_print_properties(
#    TARGETS kdb::C
#    PROPERTIES
#        INTERFACE_INCLUDE_DIRECTORIES
#        IMPORTED_LOCATION
#        IMPORTED_IMPLIB
#        IMPORTED_OBJECTS
#)
