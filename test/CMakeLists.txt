# vim: set et noai ts=4 sw=4 syntax=cmake:
set(target_name "test_q_ffi")
set(target_source_dir src)
set(target_header_dir inc)

include(GoogleTest)

# Unit test implementation
add_executable(${target_name})
target_sources(${target_name}
    PRIVATE
        ${target_source_dir}/setup.cpp          ${target_header_dir}/setup.hpp
        ${target_source_dir}/test_std_ext.cpp
        ${target_source_dir}/test_ktypes.cpp
        ${target_source_dir}/test_temporals.cpp
        ${target_source_dir}/test_kpointer.cpp
)
target_include_directories(${target_name}
    PRIVATE
        ${target_header_dir}
        CapitalEdge::q_ffi
)
target_link_libraries(${target_name}
    PRIVATE
        CapitalEdge::q_ffi
        gtest
        gtest_main
)

# Copy test dependencies
add_custom_command(
    TARGET ${target_name}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E
        copy_if_different $<TARGET_FILE:CapitalEdge::q_ffi> $<TARGET_FILE_DIR:${target_name}>
    COMMENT "Copy all dependent DLLs to the test directory."
)
#if(WIN32)
#    add_custom_command(
#        TARGET ${target_name}
#        POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E
#            copy_if_different $<TARGET_FILE:kdb::C> $<TARGET_FILE_DIR:${target_name}>
#        COMMENT "Copy kdb+ DLL to the test directory."
#    )
#endif()

# Test cases
add_test(
    NAME q_ffi_Tests
    COMMAND test_q_ffi
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
)
