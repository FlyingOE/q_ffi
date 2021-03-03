# vim: set et noai ts=4 sw=4 syntax=cmake:
cmake_minimum_required(VERSION 3.15)

if(WIN32)
    set(DLFCN_NAME dlfcn-win32)
    set(DLFCN_VERSION 1.2.0)

#[===[
    include(FetchContent)
    FetchContent_Declare(${DLFCN_NAME}
        GIT_REPOSITORY https://github.com/dlfcn-win32/dlfcn-win32.git
        GIT_TAG        v${DLFCN_VERSION}
    )
    cmake_policy(SET CMP0048 NEW)
    FetchContent_MakeAvailable(${DLFCN_NAME})
    set(dlfcn-win32_DIR "${dlfcn-win32_SOURCE_DIR}"
        CACHE INTERNAL "dlfcn-win32 source by FetchContent")
#]===]

    project(External_${DLFCN_NAME}
        VERSION ${DLFCN_VERSION}
        DESCRIPTION "An implementation of dlfcn for Windows"
        HOMEPAGE_URL "https://github.com/dlfcn-win32/dlfcn-win32"
    )
    set(target_name ${DLFCN_NAME})

    add_library(${target_name} STATIC)
    target_sources(${target_name}
        PUBLIC
            dlfcn.h
        PRIVATE
            dlfcn.c
    )
    target_include_directories(${target_name}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
    )

    add_library(dlfcn-win32::dl ALIAS ${target_name})

endif()