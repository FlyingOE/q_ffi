###########################
# To install and use libffi packages locally:
# dynamic library
#> vcpkg install libffi:x86-windows libffi:x64-windows
# static library
#> vcpkg install libffi:x86-windows-static libffi:x64-windows-static
# make installed package available (especially for Visual Studio)
#> vcpkg integrate install

# !! the find_package always try to find the dynamic library, so if the dynamic version is installed, we shall comment out the -DFFI_BUILDING
###########################
cmake_minimum_required(VERSION 3.5)

set(LIBFFI_NAME libffi)
set(LIBFFI_VERSION 3.3)

include(FetchContent)

FetchContent_Declare(${LIBFFI_NAME}
    GIT_REPOSITORY  https://github.com/libffi/libffi.git
    GIT_TAG         v${LIBFFI_VERSION}          
)

message(message(STATUS "GitHub >> Fetching ${LIBFFI_NAME}/v${LIBFFI_VERSION} ..."))

set(CMAKE_CXX_STANDARD 17)
set(USE_SYSTEM_TZ_DB   ON)

FetchContent_MakeAvailable(${LIBFFI_NAME})

message(STATUS "------ << Updated ${LIBFFI_NAME}/v${LIBFFI_VERSION} .")
