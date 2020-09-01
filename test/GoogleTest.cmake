### vim: set et noai ts=4 sw=4 :

# Download and unpack googletest at configure time
configure_file(GoogleTest_CMakeLists.txt.in GoogleTest/download/CMakeLists.txt)
execute_process(
    COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/GoogleTest/download
    RESULT_VARIABLE result
)
if(result)
    message(FATAL_ERROR "CMake step for GoogleTest failed: ${result}")
endif()
execute_process(
    COMMAND ${CMAKE_COMMAND} --build .
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/GoogleTest/download
    RESULT_VARIABLE result
)
if(result)
    message(FATAL_ERROR "Build step for GoogleTest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add GoogleTest directly to our build. This defines the gtest and gtest_main targets
add_subdirectory(
    ${CMAKE_CURRENT_BINARY_DIR}/GoogleTest/src
    ${CMAKE_CURRENT_BINARY_DIR}/GoogleTest/build
    EXCLUDE_FROM_ALL
)

# The gtest/gtest_main targets carry header search path dependencies automatically when using CMake 2.8.11 or later.
# Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
endif()

## Now simply link against gtest or gtest_main as needed. Eg
#add_executable(example example.cpp)
#target_link_libraries(example gtest_main)
#add_test(NAME example_test COMMAND example)
