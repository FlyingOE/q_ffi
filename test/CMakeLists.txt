# vim: set et noai ts=4 sw=4 syntax=cmake:
set(target_name "test_q_ffi")

# GTest
enable_testing()
include(GoogleTest)

# Unit test implementation
add_executable(${target_name})
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

# Test DLL
add_library(${target_name}_dll SHARED)
add_dependencies(${target_name}
    ${target_name}_dll
)
target_sources(${target_name}_dll
    PRIVATE
        dll/test_dll.cpp    dll/test_dll_win.hpp    dll/test_dll_linux.hpp
)

# q test scripst
set(test_scripts
    test_ffcall_add.q
    test_ffcall_args.q
)
set(test_scripts_src
    test.lib.q
)
foreach(script ${test_scripts})
    list(APPEND test_scripts_src ${script}.in)
endforeach()
set_target_properties(${target_name} PROPERTIES
    RESOURCE  "${test_scripts_src}"
)

# Copy test dependencies
add_custom_target(${target_name}.q_ffi ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:CapitalEdge::q_ffi> $<TARGET_FILE_DIR:${target_name}>
    DEPENDS CapitalEdge::q_ffi
    COMMENT "Copy all q_ffi binaries into the test directory."
    VERBATIM
)
if(WIN32)
    add_custom_target(${target_name}.kdb ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:kdb::C-exe> $<TARGET_FILE_DIR:${target_name}>
        DEPENDS kdb::C-exe
        COMMENT "Copy kdb+ runtime into the test directory."
        VERBATIM
    )
endif()
add_custom_target(${target_name}.test-scripts ALL
    COMMAND ${CMAKE_COMMAND}
            -D "test_scripts=${test_scripts}"
            -D "binary_dir=${CMAKE_CURRENT_BINARY_DIR}"
            -P test_ffcall.cmake
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${test_scripts_src}
    COMMENT "Generate q test scripts into the test directory."
    VERBATIM
)

# Test cases
add_test(
    NAME q_ffi_Tests
    COMMAND test_q_ffi
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
)
