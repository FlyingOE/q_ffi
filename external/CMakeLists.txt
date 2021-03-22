# vim: set et noai ts=4 sw=4 syntax=cmake:
cmake_minimum_required(VERSION 3.15)

add_subdirectory(q)
add_subdirectory(HowardHinnant)
add_subdirectory(libffi)
add_subdirectory(GTest)

if(WIN32)
add_subdirectory(dlfcn-win32)
endif()
