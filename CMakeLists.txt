cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0048 NEW)	# project(VERSION)
include(CMakePrintHelpers)

########################################
project(q_ffi
	VERSION 0.1
	DESCRIPTION "K9-like FFI for kdb+/q"
	HOMEPAGE_URL "https://github.com/FlyingOE/q_ffi")

# Build requirements
enable_language(CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Project global setup
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Platform detection
if(WIN32)
	if(NOT (MSVC OR MSYS OR MINGW))
		message(FATAL_ERROR
			"FIXME: Support for non-MSVC needed...")
	endif()
	set(k4_SYSTEM 119)	# ASCII for 'w'
	if(CMAKE_VS_PLATFORM_NAME STREQUAL "Win32")
		set(k4_PLATFORM "w32")
	elseif(CMAKE_VS_PLATFORM_NAME STREQUAL "x64")
		set(k4_PLATFORM "w64")
	else()
		message(FATAL_ERROR
			"Failed to detect Visual Studio platform: CMAKE_VS_PLATFORM_NAME=${CMAKE_VS_PLATFORM_NAME}")
	endif()
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
elseif(UNIX AND NOT APPLE)
	set(k4_SYSTEM 108)	# ASCII for 'l'
	if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86")
		set(k4_PLATFORM "l32")
	elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
		set(k4_PLATFORM "l64")
	else()
		message(FATAL_ERROR
			"Failed to detect Linux platform: CMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}")
	endif()
	message(WARNING
		"FIXME: Linux support has not been verified yet.")
elseif(APPLE)
	set(k4_SYSTEM 109)	# ASCII for 'm'
	message(FATAL_ERROR
		"FIXME: macOS not support yet!")
else()
	message(FATAL_ERROR
		"FIXME: Unsupport target platform!")
endif()

cmake_print_variables(k4_SYSTEM)
cmake_print_variables(k4_PLATFORM)

# Project contents
configure_file(q_ffi_config.h.in q_ffi.h @ONLY)
include_directories(${PROJECT_BINARY_DIR})

add_subdirectory(q_ffi)
