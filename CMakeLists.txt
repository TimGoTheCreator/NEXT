cmake_minimum_required(VERSION 3.10)

project(NEXT)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

option(NEXT_FP32 "Use 32-bit floats" OFF)
option(NEXT_FP64 "Use 64-bit floats" ON)
option(NEXT_SIMD32 "Use 32-bit SIMD" OFF)
option(NEXT_SIMD64 "Use 64-bit SIMD" OFF)

if(NEXT_FP32)
    add_compile_definitions(NEXT_FP32)
elseif(NEXT_FP64)
    add_compile_definitions(NEXT_FP64)
elseif(NEXT_SIMD32)
    add_compile_definitions(NEXT_SIMD32)
elseif(NEXT_SIMD64)
    add_compile_definitions(NEXT_SIMD64)
else()
    message(FATAL_ERROR "You must enable either NEXT_FP32 or NEXT_FP64")
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(next ${SRC_FILES})