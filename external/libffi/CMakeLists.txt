###########################
# To install and use libffi packages locally:
# dynamic library
#> vcpkg install libffi:x86-windows libffi:x64-windows
#> vcpkg integrate install
# static libraray
#> vcpkg install libffi:x[86|64]-windows-static
#!! for static lib, the following definition is necessary https://github.com/microsoft/vcpkg/issues/1338
#> cmake -DCMAKE_TOOCHAIN_FILE=** -DVCPKG_TARGET_TRIPLET=x[86|64]-windows-static

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
