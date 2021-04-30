# vim: set et noai ts=4 sw=4 syntax=cmake:
set(target_name       q_ffi)
set(target_source_dir src)
set(target_header_dir inc)

# Embed Git hash into binary
find_package(Git REQUIRED)
execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
    OUTPUT_VARIABLE q_ffi_GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE )
configure_file(q_ffi_config.h.in q_ffi_config.h @ONLY)

find_package(date CONFIG REQUIRED)

# Library contents
set(q_ffi_BINDING
    q_ffi.q
)
set(q_ffi_PUBLIC_API
    ${target_header_dir}/q_ffi.h
    ${target_header_dir}/std_ext.hpp
    ${target_header_dir}/ktypes.hpp
    ${target_header_dir}/ktype_traits.hpp
    ${target_header_dir}/ktype_traits_io.hpp
    ${target_header_dir}/kerror.hpp
    ${target_header_dir}/kpointer.hpp
    ${target_header_dir}/type_convert.hpp
)
set(q_ffi_HEADERS
    ${CMAKE_CURRENT_BINARY_DIR}/q_ffi_config.h
    ${target_header_dir}/ffi_api.h
    ${target_header_dir}/version.hpp
    ${target_header_dir}/dlloader.hpp
    ${target_header_dir}/ffargs.hpp
    ${target_header_dir}/ffcall.hpp
)
set(q_ffi_SOURCES
    ${target_source_dir}/dllmain.cpp
    ${target_source_dir}/ffi_api.cpp
    ${target_source_dir}/ktypes.cpp
    ${target_source_dir}/ktype_traits.cpp
    ${target_source_dir}/kerror.cpp
    ${target_source_dir}/type_convert.cpp
    ${target_source_dir}/dlloader.cpp
    ${target_source_dir}/ffargs.cpp
    ${target_source_dir}/ffcall.cpp
)
set(q_ffi_ALWAYS_BUILD
    ${target_source_dir}/version.cpp
)

source_group("q"              FILES ${q_ffi_BINDING})
source_group("Public Headers" FILES ${q_ffi_PUBLIC_API})
source_group("Headers"        FILES ${q_ffi_HEADERS})
source_group("Sources"        FILES ${q_ffi_SOURCES} ${q_ffi_ALWAYS_BUILD})

# Library build definition
add_library(${target_name} SHARED)
target_sources(${target_name}
    PUBLIC
        ${q_ffi_PUBLIC_API}
    PRIVATE
        ${q_ffi_HEADERS}
        ${q_ffi_SOURCES}
        ${q_ffi_ALWAYS_BUILD}
)
set_target_properties(${target_name} PROPERTIES
    DEBUG_POSTFIX "d"
    RESOURCE      "${q_ffi_BINDING}"
    PUBLIC_HEADER "${q_ffi_PUBLIC_API}"
)
target_compile_features(${target_name}
    PUBLIC
        cxx_std_17
)
target_compile_definitions(${target_name}
    PUBLIC
        k4_SYSTEM=$ENV{k4_SYSTEM}
        k4_PLATFORM=$ENV{k4_PLATFORM}
)
target_include_directories(${target_name}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${target_header_dir}>
        $<INSTALL_INTERFACE:${target_header_dir}>
        kdb::C-dll
    PRIVATE
        libffi::static
)
target_link_libraries(${target_name}
    PUBLIC
        kdb::C-dll
        date::date
    PRIVATE
        libffi::static
)
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    get_target_property(dll_suffix ${target_name} DEBUG_POSTFIX)
else()
    set(dll_suffix "")
endif()
add_custom_target(${target_name}.binding ALL
    COMMAND ${CMAKE_COMMAND}
            -D "q_script=${q_ffi_BINDING}"
            -D "binary_dir=${CMAKE_CURRENT_BINARY_DIR}"
            -D "dll_suffix=${dll_suffix}"
            -P q_ffi.cmake
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${q_ffi_BINDING}
    COMMENT "Generate q script binding."
    VERBATIM
)

# dlfcn-win32 to emulate dlopen/dlsym
if($ENV{k4_SYSTEM} STREQUAL "w")
    target_include_directories(${target_name}
        PRIVATE
            dlfcn-win32::dl
    )
    target_link_libraries(${target_name}
        PRIVATE
            dlfcn-win32::dl
    )
endif()

if(WIN32)
    # 1) Remove offending default compiler options
    remove_definitions(_MBCS)
    string(REPLACE "/W3" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    # 2) Add desired compiler options
    target_compile_definitions(${target_name}
        PUBLIC
            _UNICODE
            _SILENCE_CXX17_UNCAUGHT_EXCEPTION_DEPRECATION_WARNING
    )
    target_compile_options(${target_name}
        PUBLIC
            /utf-8      # source file encoding
            /MP         # multi-threaded build
            /EHsc       # standard C++ exception handling
        PRIVATE
            /W4 /WX     # warning level - compulsory
    )
elseif(UNIX AND NOT APPLE)
    target_compile_options(${target_name}
        PRIVATE
            -Wall -Wextra -Wpedantic -Werror
    )
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target_name}
            PUBLIC
                -Wno-unknown-pragmas         # g++ bails upon `#pragma region'
                -Wno-deprecated-declarations # std::uncaught_exception deprecated
            PRIVATE
                -Wno-unused-parameter
                -Wno-unused-variable
                -fdiagnostics-color=always
        )
    endif()
    target_link_libraries(${target_name}
        INTERFACE
            dl
    )
elseif(APPLE)
    message(FATAL_ERROR "FIXME: macOS not yet supported!")
else()
    message(FATAL_ERROR "FIXME: unsupported target platform!")
endif()

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE   ON)

# Always rebuild version source to embed timestamp
add_custom_target(update_version ALL
    COMMAND ${CMAKE_COMMAND} -E touch ${q_ffi_ALWAYS_BUILD}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Touch source to force embedding build info into binary."
    VERBATIM
)
add_dependencies(${target_name}
    update_version
)

# Installation steps
install(TARGETS ${target_name}
    RUNTIME       DESTINATION bin/$ENV{k4_PLATFORM}
    ARCHIVE       DESTINATION lib/$ENV{k4_PLATFORM}
    PUBLIC_HEADER DESTINATION include
)
foreach(script ${q_ffi_BINDING})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${script}
        DESTINATION bin
    )
endforeach()

####################
add_library(CapitalEdge::q_ffi ALIAS ${target_name})
