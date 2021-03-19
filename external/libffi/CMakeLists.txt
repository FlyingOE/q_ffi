# vim: set et noai ts=4 sw=4 syntax=cmake:
cmake_minimum_required(VERSION 3.15)

include(CMakePrintHelpers)

########################################
project(libffi_msvc
    VERSION 3.3
    DESCRIPTION "libffi (MSVC build)"
    HOMEPAGE_URL "https://github.com/libffi/libffi"
)

# Platform detection
if(WIN32)
    if(NOT (MSVC OR MSYS OR MINGW))
        message(FATAL_ERROR "FIXME: Support non-MSVC?")
    endif()
    set(libffi_SYSTEM "msvc")
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(libffi_PLATFORM "x86")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(libffi_PLATFORM "x64")
    else()
        message(FATAL_ERROR "Unknown sizeof(void*): CMAKE_SIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P}")
    endif()
elseif(UNIX AND NOT APPLE)
    set(libffi_SYSTEM "linux")
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86")
        set(libffi_PLATFORM "x86")
    elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        set(libffi_PLATFORM "x64")
    else()
        message(FATAL_ERROR "Failed to detect Linux platform: CMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}")
    endif()
elseif(APPLE)
    set(libffi_SYSTEM "mac")
    message(FATAL_ERROR "FIXME: macOS not support yet!")
else()
    message(FATAL_ERROR "FIXME: Unsupport target platform!")
endif()

# Library contents
set(libffi_IMPORTS ${CMAKE_CURRENT_SOURCE_DIR}/${libffi_SYSTEM}/${libffi_PLATFORM})
cmake_print_variables(libffi_IMPORTS)

add_library(libffi_static STATIC IMPORTED GLOBAL)
target_include_directories(libffi_static
    INTERFACE ${libffi_IMPORTS}/include
)
target_compile_definitions(libffi_static
    INTERFACE FFI_BUILDING
)

add_library(libffi_shared SHARED IMPORTED GLOBAL)
target_include_directories(libffi_shared
    INTERFACE ${libffi_IMPORTS}/include
)
target_compile_definitions(libffi_shared
    PRIVATE #FFI_BUILDING_DLL
)

if(libffi_SYSTEM STREQUAL "msvc")
    set_target_properties(libffi_static PROPERTIES
        IMPORTED_LOCATION ${libffi_IMPORTS}/lib/libffi_convenience.lib
    )
    set_target_properties(libffi_shared PROPERTIES
        IMPORTED_LOCATION ${libffi_IMPORTS}/lib/libffi-7.dll
        IMPORTED_IMPLIB ${libffi_IMPORTS}/lib/libffi-7.lib
    )

elseif(libffi_SYSTEM STREQUAL "linux")
    set_target_properties(libffi_static PROPERTIES
        IMPORTED_LOCATION ${libffi_IMPORTS}/lib/???
    )
    set_target_properties(libffi_shared PROPERTIES
        IMPORTED_LOCATION ${libffi_IMPORTS}/lib/???
        IMPORTED_IMPLIB ${libffi_IMPORTS}/lib/???
    )
    messsage(FATAL_ERROR "FIXME: add Linux libraries...")

elseif(libffi_SYSTEM STREQUAL "mac")
    # FIXME
else()
    # FIXME
endif()

add_library(libffi::static ALIAS libffi_static)
add_library(libffi::shared ALIAS libffi_shared)

cmake_print_properties(
    TARGETS libffi::shared
    PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES
        IMPORTED_LOCATION
        IMPORTED_IMPLIB
)
cmake_print_properties(
    TARGETS libffi::static
    PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES
        IMPORTED_LOCATION
        IMPORTED_IMPLIB
)
