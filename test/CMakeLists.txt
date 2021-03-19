# vim: set et noai ts=4 sw=4 syntax=cmake:
set(target_name "test_q_ffi")
set(source_dir src)
set(header_dir inc)

# GTest
enable_testing()
include(GoogleTest)

# Unit test implementation
add_executable(${target_name})
target_sources(${target_name}
    PRIVATE
        ${source_dir}/setup.cpp     ${header_dir}/setup.hpp
        ${source_dir}/test_std_ext.cpp
        ${source_dir}/test_libffi.cpp
        ${source_dir}/test_ktypes.cpp
        ${source_dir}/test_temporals.cpp
        ${source_dir}/test_kpointer.cpp
        ${source_dir}/test_type_convert.cpp
)
target_include_directories(${target_name}
    PRIVATE
        ${header_dir}
        kdb::C-exe
        libffi::static
        CapitalEdge::q_ffi
)
target_link_libraries(${target_name}
    PRIVATE
        gtest       gtest_main
        kdb::C-exe
        libffi::static
        CapitalEdge::q_ffi
)

# Extra test assets
add_library(${target_name}_dll SHARED)
target_sources(${target_name}_dll
    PRIVATE
        ${source_dir}/test_libffi_dll.cpp
)
add_dependencies(${target_name}
    ${target_name}_dll
)

# Copy test dependencies
add_custom_command(
    TARGET ${target_name}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E
        copy_if_different $<TARGET_FILE:CapitalEdge::q_ffi> $<TARGET_FILE_DIR:${target_name}>
    COMMENT "Copy all dependent DLLs to the test directory."
)
if(WIN32)
    add_custom_command(
        TARGET ${target_name}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E
            copy_if_different $<TARGET_FILE:kdb::C-exe> $<TARGET_FILE_DIR:${target_name}>
        COMMENT "Copy kdb+ DLL to the test directory."
    )
endif()

# Test cases
add_test(
    NAME q_ffi_Tests
    COMMAND test_q_ffi
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
)
