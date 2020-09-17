# vim: set et noai ts=4 sw=4 syntax=cmake:
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
    link_libraries(c)       # Windows build requires `c.lib'
    set(q_ffi_DEPENDENTS )
#    set(MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
elseif(k4_SYSTEM STREQUAL "l")
    set(q_ffi_DEPENDENTS    # Linux build requires `c.o'
        ${q_ffi_DEPENDENCIES}/q/${k4_PLATFORM}/c.o
    )
endif()

# Build targets
add_executable(test_q_ffi
    "setup.cpp"    "setup.hpp"
    "test_std_ext.cpp"
    "test_ktypes.cpp"
    "test_temporals.cpp"
    "test_kpointer.cpp"
    ${q_ffi_DEPENDENTS}
)
target_link_libraries(test_q_ffi
    q_ffi
    gtest_main
)

# Copy test dependencies
if(k4_SYSTEM STREQUAL "w")
    add_custom_command(
        TARGET test_q_ffi
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E
            copy ${CMAKE_CURRENT_BINARY_DIR}/../q_ffi/q_ffi.dll ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E
            copy ${q_ffi_DEPENDENCIES}/q/${k4_PLATFORM}/c.dll ${CMAKE_CURRENT_BINARY_DIR}
    )
else()
    add_custom_command(
        TARGET test_q_ffi
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E
            copy ${CMAKE_CURRENT_BINARY_DIR}/../q_ffi/libq_ffi.so ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()

# Test cases
add_test(
    NAME q_ffi_Tests
    COMMAND test_q_ffi
)
