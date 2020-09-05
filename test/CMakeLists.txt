### vim: set et noai ts=4 sw=4 syntax=cmake:
set(CTEST_USE_LAUNCHERS ON)

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
    link_libraries(c)    # Windows build requires `c.lib'
#    set(MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
elseif(k4_SYSTEM STREQUAL "l")
    link_libraries(c.o)  # Linux build requires `c.o'
endif()

# Build targets
add_executable(test_q_ffi
    test_std_ext.cpp
    test_types.cpp
    test_pointer.cpp
)

target_link_libraries(test_q_ffi
    q_ffi
    gtest_main
)

# Copy test dependencies
if(k4_SYSTEM STREQUAL "w")
    add_custom_target(copy_test_deps ALL
        COMMAND ${CMAKE_COMMAND} -E
            copy ${q_ffi_DEPENDENCIES}/q/${k4_PLATFORM}/c.dll ${CMAKE_CURRENT_BINARY_DIR}
    )
    add_dependencies(test_q_ffi copy_test_deps)
endif()

# Test cases
add_test(
    NAME q_ffi_Tests
    COMMAND test_q_ffi
)
