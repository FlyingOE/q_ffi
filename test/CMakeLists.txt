# vim: set et noai ts=4 sw=4 syntax=cmake:
set(target_name "test_q_ffi")

# GTest
enable_testing()
include(GoogleTest)

# Unit test implementation
add_executable(${target_name})
set_target_properties(${target_name} PROPERTIES
    RESOURCE  test_ffcall.q
)
target_sources(${target_name}
    PRIVATE
        setup.cpp     setup.hpp
        test_std_ext.cpp
        test_kpointer.cpp
        test_ktypes.cpp
        test_temporals.cpp
        test_type_convert.cpp
        test_libffi.cpp     test_libffi_win.hpp     test_libffi_linux.hpp
        test_ffcall.cpp
)
target_include_directories(${target_name}
    PRIVATE
        ${header_dir}
        ${target_name}_dll
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
if(WIN32)
    target_link_options(${target_name}
        PRIVATE
            /NODEFAULTLIB:MSVCRT
    )
    target_include_directories(${target_name}
        PRIVATE
            dlfcn-win32::dl
    )
    target_link_libraries(${target_name}
        PRIVATE
            dlfcn-win32::dl
    )
endif()

# Extra test assets
add_library(${target_name}_dll SHARED)
add_dependencies(${target_name}
    ${target_name}_dll
)
target_sources(${target_name}_dll
    PRIVATE
        dll/test_dll.cpp    dll/test_dll_win.hpp    dll/test_dll_linux.hpp
)

# Copy test dependencies
add_custom_command(
    TARGET ${target_name}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:CapitalEdge::q_ffi> $<TARGET_FILE_DIR:${target_name}>
    COMMENT "Copy all dependent DLLs to the test directory."
)
if(WIN32)
    add_custom_command(
        TARGET ${target_name}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:kdb::C-exe> $<TARGET_FILE_DIR:${target_name}>
        COMMENT "Copy kdb+ DLL to the test directory."
    )
endif()
add_custom_command(
    TARGET ${target_name}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_PROPERTY:${target_name},SOURCE_DIR>/$<TARGET_PROPERTY:${target_name},RESOURCE>
            $<TARGET_FILE_DIR:${target_name}>
    COMMENT "Copy all dependent resources to the test directory."
)

# Test cases
add_test(
    NAME q_ffi_Tests
    COMMAND test_q_ffi
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
)
