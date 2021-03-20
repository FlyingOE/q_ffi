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
foreach(X IN ITEMS k4_SYSTEM k4_PLATFORM)
    cmake_print_variables(${X})
    set(ENV{${X}} ${${X}})
endforeach()

if(k4_SYSTEM STREQUAL "w")
    # DLL to be loaded into q.exe
    add_library(kdb_C_DLL STATIC IMPORTED GLOBAL)
    target_include_directories(kdb_C_DLL
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_target_properties(kdb_C_DLL PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/${k4_PLATFORM}/q.lib
    )

    # stand-alone EXE
    add_library(kdb_C_EXE SHARED IMPORTED GLOBAL)
    target_include_directories(kdb_C_EXE
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_target_properties(kdb_C_EXE PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/${k4_PLATFORM}/c.dll
        IMPORTED_IMPLIB   ${CMAKE_CURRENT_SOURCE_DIR}/${k4_PLATFORM}/c.lib
    )

elseif(k4_SYSTEM STREQUAL "l")
    # SO to be loaded into q
    add_library(kdb_C_DLL INTERFACE IMPORTED GLOBAL)
    target_include_directories(kdb_C_DLL
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_compile_options(kdb_C_DLL
        INTERFACE
            -fPIC
    )

    # stand-alone ELF
    add_library(kdb_C_EXE OBJECT IMPORTED GLOBAL)
    target_include_directories(kdb_C_EXE
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_target_properties(kdb_C_EXE PROPERTIES
        IMPORTED_OBJECTS ${CMAKE_CURRENT_SOURCE_DIR}/${k4_PLATFORM}/c.o
        #IMPORTED_OBJECTS ${CMAKE_CURRENT_SOURCE_DIR}/${k4_PLATFORM}/e.o
    )

elseif(k4_SYSTEM STREQUAL "m")
    # FIXME
else()
    # FIXME
endif()

# Alias for export
add_library(kdb::C-dll ALIAS kdb_C_DLL)
add_library(kdb::C-exe ALIAS kdb_C_EXE)

if(${CMAKE_VERSION} VERSION_GREATER "3.18")
    cmake_print_properties(
        TARGETS kdb::C-dll
        PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES
            IMPORTED_LOCATION
            IMPORTED_IMPLIB
            IMPORTED_OBJECTS
    )
    cmake_print_properties(
        TARGETS kdb::C-exe
        PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES
            IMPORTED_LOCATION
            IMPORTED_IMPLIB
            IMPORTED_OBJECTS
    )
endif()
