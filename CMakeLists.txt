cmake_minimum_required(VERSION 3.10)

project(NEXT)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Precision / SIMD mode options
option(NEXT_FP32        "Use 32-bit floats (scalar)"        OFF)
option(NEXT_FP64        "Use 64-bit floats (scalar)"        ON)
option(NEXT_SIMD32      "Use 32-bit SIMD (SSE)"             OFF)
option(NEXT_SIMD64      "Use 64-bit SIMD (SSE)"             OFF)
option(NEXT_AVX512_32   "Use 32-bit SIMD (AVX-512)"         OFF)
option(NEXT_AVX512_64   "Use 64-bit SIMD (AVX-512)"         OFF)

# Count enabled modes
math(EXPR NEXT_MODE_COUNT
    "${NEXT_FP32} + ${NEXT_FP64} + ${NEXT_SIMD32} + ${NEXT_SIMD64} + ${NEXT_AVX512_32} + ${NEXT_AVX512_64}"
)

if(NEXT_MODE_COUNT EQUAL 0)
    message(FATAL_ERROR "You must enable one precision/SIMD mode.")
elseif(NEXT_MODE_COUNT GREATER 1)
    message(FATAL_ERROR "You must enable ONLY ONE of: NEXT_FP32, NEXT_FP64, NEXT_SIMD32, NEXT_SIMD64, NEXT_AVX512_32, NEXT_AVX512_64.")
endif()

# Apply compile definitions and flags
if(NEXT_FP32)
    add_compile_definitions(NEXT_FP32)

elseif(NEXT_FP64)
    add_compile_definitions(NEXT_FP64)

elseif(NEXT_SIMD32)
    add_compile_definitions(NEXT_SIMD32)
    add_compile_options(-msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2)

elseif(NEXT_SIMD64)
    add_compile_definitions(NEXT_SIMD64)
    add_compile_options(-msse2 -msse3 -mssse3 -msse4.1 -msse4.2)

elseif(NEXT_AVX512_32)
    add_compile_definitions(NEXT_AVX512_32)
    add_compile_options(-mavx512f -mavx512dq -mavx512vl)

elseif(NEXT_AVX512_64)
    add_compile_definitions(NEXT_AVX512_64)
    add_compile_options(-mavx512f -mavx512dq -mavx512vl)

endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(next ${SRC_FILES})