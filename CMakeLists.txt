cmake_minimum_required(VERSION 3.10)

project(NEXT)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

option(NEXT_FP32 "Use 32-bit floats" OFF)
option(NEXT_FP64 "Use 64-bit floats" ON)

if(NEXT_FP32)
    add_compile_definitions(NEXT_FP32)
elseif(NEXT_FP64)
    add_compile_definitions(NEXT_FP64)
else()
    message(FATAL_ERROR "You must enable either NEXT_FP32 or NEXT_FP64")
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(next ${SRC_FILES})