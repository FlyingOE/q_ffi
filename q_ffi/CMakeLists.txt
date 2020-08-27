include_directories(../inc)
link_directories(../lib/${k4_PLATFORM})
if(k4_SYSTEM 119)
	link_libraries(q)	# Windows build requires `q.lib'
#	set(MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	set(MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

file(GLOB q_ffi_HEADERS "*.h" "*.hpp")
file(GLOB q_ffi_SOURCES "*.cpp")
set(q_ffi_ALL ${q_ffi_SOURCES} ${q_ffi_HEADERS})
file(GLOB q_ffi_SCRIPTS "*.q")

source_group("" FILES ${q_ffi_ALL})

#include_directories(.)

add_library(q_ffi SHARED ${q_ffi_ALL})

install(TARGETS q_ffi
	RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/deploy/${k4_PLATFORM})
install(FILES ${q_ffi_SCRIPTS}
	DESTINATION ${PROJECT_SOURCE_DIR}/dist)
