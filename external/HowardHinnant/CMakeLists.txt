# vim: set et noai ts=4 sw=4 syntax=cmake:
cmake_minimum_required(VERSION 3.14)

set(HH_DATE_NAME    HowardHinnant_date)
set(HH_DATE_VERSION 3.0.0)

# @ref https://github.com/HowardHinnant/date/blob/master/CMakeLists.txt
# @ref https://cmake.org/cmake/help/git-stage/module/FetchContent.html
include(FetchContent)

FetchContent_Declare(${HH_DATE_NAME}
    GIT_REPOSITORY https://github.com/HowardHinnant/date.git
    GIT_TAG        v${HH_DATE_VERSION}
)

message(STATUS "GitHub >> Fetching ${HH_DATE_NAME}/v${HH_DATE_VERSION} ...")
set(CMAKE_CXX_STANDARD 17)
set(USE_SYSTEM_TZ_DB   ON)
FetchContent_MakeAvailable(${HH_DATE_NAME})
message(STATUS "       << Updated ${HH_DATE_NAME}/v${HH_DATE_VERSION} .")
