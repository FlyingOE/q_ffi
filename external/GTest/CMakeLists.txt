# vim: set et noai ts=4 sw=4 syntax=cmake:
cmake_minimum_required(VERSION 3.14)

set(GTEST_NAME GoogleTest)
set(GTEST_VERSION 1.10.0)

# @ref https://cliutils.gitlab.io/modern-cmake/chapters/testing/googletest.html#fn_1
# @ref https://cmake.org/cmake/help/git-stage/module/FetchContent.html
include(FetchContent)

FetchContent_Declare(${GTEST_NAME}
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-${GTEST_VERSION}
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(BUILD_GTEST ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(${GTEST_NAME})

#[===[
FetchContent_GetProperties(${GTEST_NAME})
string(TOLOWER "${GTEST_NAME}" lcname)
if(NOT ${lcname}_POPULATED)
    FetchContent_Populate(${GTEST_NAME})
    add_subdirectory(${${lcname}_SOURCE_DIR} ${${lcname}_BINARY_DIR})
endif()
#]===]
