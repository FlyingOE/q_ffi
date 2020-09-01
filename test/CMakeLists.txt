### vim: set et noai ts=4 sw=4 syntax=cmake:

include(GoogleTest.cmake)

# Build dependencies
include_directories(
    ${PROJECT_SOURCE_DIR}/q_ffi
    ${q_ffi_DEPENDENCIES}/q
    ${gtest_INCLUDE_DIRS}
)
link_directories(
    ${q_ffi_DEPENDENCIES}/q/${k4_PLATFORM}
)
if(k4_SYSTEM STREQUAL "w")
    link_libraries(q)    # Windows build requires `q.lib'
#    set(MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Build targets
add_executable(unit_test
    test_pointer.cpp
)

target_link_libraries(unit_test
    q_ffi
    gtest_main
)

# Test cases
add_test(
    NAME UnitTests
    COMMAND unit_test
)
