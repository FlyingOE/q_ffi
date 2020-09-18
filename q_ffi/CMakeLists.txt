# vim: set et noai ts=4 sw=4 syntax=cmake:

# Build dependencies
include_directories(
    ${q_ffi_DEPENDENCIES}/q
)
link_directories(
    ${q_ffi_DEPENDENCIES}/q/${k4_PLATFORM}
)
if(k4_SYSTEM STREQUAL "w")
    link_libraries(q)    # Windows build requires `q.lib'
#    set(MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Source files
set(q_ffi_Q_SCRIPTS
    "q_ffi.q"
)
set(q_ffi_PUBLIC_HEADERS
    "${PROJECT_BINARY_DIR}/q_ffi.h"
    "k_compat.h"
    "std_ext.hpp"
    "ktypes.hpp"
    "ktype_traits.hpp"
    "kpointer.hpp"
    "kerror.hpp"
)
set(q_ffi_HEADERS
    "version.hpp"
)
set(q_ffi_SOURCES
    "ffi.cpp"
    "ktypes.cpp"
    "ktype_traits.cpp"
    "kerror.cpp"
)
set(q_ffi_ALWAYS_REBUILD
    "version.cpp"
)

source_group("q"              ${q_ffi_Q_SCRIPTS})
source_group("Public Headers" ${q_ffi_PUBLIC_HEADERS})
source_group("Headers"        ${q_ffi_HEADERS})
source_group("Sources"        ${q_ffi_SOURCES} ${q_ffi_ALWAYS_REBUILD})

# Build targets
add_library(q_ffi SHARED
    ${q_ffi_SOURCES}
    ${q_ffi_ALWAYS_REBUILD}
    ${q_ffi_HEADERS}
    ${q_ffi_PUBLIC_HEADERS}
)

set_target_properties(q_ffi PROPERTIES
    RESOURCE      "${q_ffi_Q_SCRIPTS}"
    PUBLIC_HEADER "${q_ffi_PUBLIC_HEADERS}"
)

# Always rebuild version source to embed timestamp
add_custom_target(update_version ALL
    COMMAND ${CMAKE_COMMAND} -E touch ${q_ffi_ALWAYS_REBUILD}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
add_dependencies(q_ffi update_version)

# Installation steps
install(TARGETS q_ffi
    RUNTIME       DESTINATION ${q_ffi_DISTRIBUTABLES}/bin/${k4_PLATFORM}
    RESOURCE      DESTINATION ${q_ffi_DISTRIBUTABLES}/bin
    ARCHIVE       DESTINATION ${q_ffi_DISTRIBUTABLES}/lib/${k4_PLATFORM}
    PUBLIC_HEADER DESTINATION ${q_ffi_DISTRIBUTABLES}/include
)
