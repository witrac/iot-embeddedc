cmake_minimum_required(VERSION 2.8)
project(iot-embeddedc)
enable_testing()

SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage -fPIC ")
SET(CMAKE_C_FLAGS "-g -O0 -Wall -W -fprofile-arcs -ftest-coverage -fPIC ")
SET(CMAKE_EXE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage ")

add_subdirectory(src)
add_subdirectory(lib)
add_subdirectory(test)
