# vim: set et noai ts=4 sw=4 syntax=cmake:
set(target_name "q_ffi_Tests")
set(target_test "${target_name}_dll")

# GoogleTest
find_package(GTest CONFIG REQUIRED)

# libffi
find_package(libffi CONFIG REQUIRED)

# Test DLL
add_library(${target_test} SHARED
    dll/test_dll.h
    dll/test_dll.cpp    dll/test_dll_win.hpp    dll/test_dll_linux.hpp
)
target_compile_features(${target_test}
    PRIVATE
        cxx_std_17
)

set(test_dll "${target_test}")
configure_file(dll/test_dll.h.in dll/test_dll.h @ONLY)
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# Unit test implementation
add_executable(${target_name}
    setup.cpp     setup.hpp
    test_std_ext.cpp
    test_kpointer.cpp
    test_ktypes.cpp
    test_temporals.cpp
    test_type_convert.cpp
    test_libffi.cpp     test_libffi_win.hpp     test_libffi_linux.hpp
    test_ffcall.cpp
)
add_dependencies(${target_name} ${target_test})
target_include_directories(${target_name}
    PRIVATE
        kdb::C-exe
        libffi
        CapitalEdge::q_ffi
)
target_link_libraries(${target_name}
    PRIVATE
        GTest::gtest    GTest::gtest_main
        kdb::C-exe
        libffi
        CapitalEdge::q_ffi
)
if(WIN32)
    target_include_directories(${target_name}
        PRIVATE
            dlfcn-win32::dl
    )
    target_link_libraries(${target_name}
        PRIVATE
            dlfcn-win32::dl
    )
endif()

# q test scripst
set(test_scripts
    q/test.lib.q
    q/test_ffcall_add.q
    q/test_ffcall_adds.q
    q/test_ffcall_args.q
    q/test_ffcall_vars.q
    q/test_ffcall_neg.q
)
set_target_properties(${target_name} PROPERTIES
    RESOURCE  "${test_scripts}"
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
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    get_target_property(dll_suffix CapitalEdge::q_ffi DEBUG_POSTFIX)
else()
    set(dll_suffix "")
endif()
add_custom_target(${target_name}.q-scripts ALL
    COMMAND ${CMAKE_COMMAND}
            -D "test_scripts=${test_scripts}"
            -D "binary_dir=${CMAKE_CURRENT_BINARY_DIR}"
            -D "test_dll=${target_test}"
            -D "dll_suffix=${dll_suffix}"
            -P tests.cmake
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${test_scripts_src}
    COMMENT "Generate q test scripts into the test directory."
    VERBATIM
)

# Test cases
include(GoogleTest)
gtest_discover_tests(${target_name}
    EXTRA_ARGS --gtest_shuffle --gtest_color=yes
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
    DISCOVERY_MODE PRE_TEST     # allow other post-build events to take place first
)
