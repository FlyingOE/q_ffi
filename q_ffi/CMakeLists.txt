### vim: set et noai ts=4 sw=4 syntax=cmake:

# Build configurations
include_directories(
	${q_ffi_DEPENDENCIES}/q
	${q_ffi_DEPENDENCIES}/magic_enum/include)
link_directories   (${q_ffi_DEPENDENCIES}/q/${k4_PLATFORM})
if(k4_SYSTEM STREQUAL "w")
	link_libraries(q)	# Windows build requires `q.lib'
#	set(MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	set(MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Source files
set(q_ffi_Q_SCRIPTS
	"q_ffi.q")
set(q_ffi_PUBLIC_HEADERS
	"k_compat.h"
	"${PROJECT_BINARY_DIR}/q_ffi.h"
	"types.hpp")
set(q_ffi_HEADERS
	"ffi.h"
	"version.hpp")
set(q_ffi_SOURCES
	"ffi.cpp"
	"types.cpp")
set(q_ffi_VERSION_SOURCE
	"version.cpp")

source_group("q"              ${q_ffi_Q_SCRIPTS})
source_group("Public Headers" ${q_ffi_PUBLIC_HEADERS})
source_group("Headers"        ${q_ffi_HEADERS})
source_group("Sources"        ${q_ffi_SOURCES} ${q_ffi_VERSION_SOURCE})

# Build target
add_library(q_ffi SHARED
	${q_ffi_SOURCES}
	${q_ffi_VERSION_SOURCE}
	${q_ffi_HEADERS}
	${q_ffi_PUBLIC_HEADERS})

set_target_properties(q_ffi PROPERTIES
	RESOURCE      "${q_ffi_Q_SCRIPTS}"
	PUBLIC_HEADER "${q_ffi_PUBLIC_HEADERS}")

# Build dependencies
if(WIN32)
	add_custom_target(update_version ALL
		COMMAND CD "${CMAKE_CURRENT_SOURCE_DIR}"
		COMMAND COPY ${q_ffi_VERSION_SOURCE}+,, ${q_ffi_VERSION_SOURCE})	# `touch' in Windows
else()
	add_custom_target(update_version ALL
		COMMAND cd "${CMAKE_CURRENT_SOURCE_DIR}"
		COMMAND touch ${q_ffi_VERSION_SOURCE})
endif()

add_dependencies(q_ffi
	magic_enum
	update_version)

# Installation steps
install(TARGETS q_ffi
	RUNTIME       DESTINATION ${q_ffi_DISTRIBUTABLES}/bin/${k4_PLATFORM}
	RESOURCE      DESTINATION ${q_ffi_DISTRIBUTABLES}/bin
	ARCHIVE       DESTINATION ${q_ffi_DISTRIBUTABLES}/lib/${k4_PLATFORM}
	PUBLIC_HEADER DESTINATION ${q_ffi_DISTRIBUTABLES}/include)