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
	"${PROJECT_BINARY_DIR}/q_ffi.h")
set(q_ffi_HEADERS
	"ffi.h"
	"misc.hpp")
set(q_ffi_SOURCES
	"ffi.cpp"
	"misc.cpp")

source_group(q       ${q_ffi_Q_SCRIPTS})
source_group(Public  ${q_ffi_PUBLIC_HEADERS})
source_group(Headers ${q_ffi_HEADERS})
source_group(Sources ${q_ffi_SOURCES})

# Build target
add_library(q_ffi SHARED
	${q_ffi_SOURCES}
	${q_ffi_HEADERS}
	${q_ffi_PUBLIC_HEADERS})
add_dependencies(q_ffi magic_enum)
set_target_properties(q_ffi PROPERTIES
	RESOURCE      "${q_ffi_Q_SCRIPTS}"
	PUBLIC_HEADER "${q_ffi_PUBLIC_HEADERS}")

# Installation steps
install(TARGETS q_ffi
	RUNTIME       DESTINATION ${q_ffi_DISTRIBUTABLES}/bin/${k4_PLATFORM}
	RESOURCE      DESTINATION ${q_ffi_DISTRIBUTABLES}/bin
	ARCHIVE       DESTINATION ${q_ffi_DISTRIBUTABLES}/lib/${k4_PLATFORM}
	PUBLIC_HEADER DESTINATION ${q_ffi_DISTRIBUTABLES}/include)