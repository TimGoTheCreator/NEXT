cmake_minimum_required(VERSION 3.10)

project(NEXT)

# ============================
# Output directories
# ============================
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# ============================
# Precision options (scalar only)
# ============================
option(NEXT_FP32 "Use 32-bit floats (scalar)" OFF)
option(NEXT_FP64 "Use 64-bit floats (scalar)" ON)

set(NEXT_MODES
    NEXT_FP32
    NEXT_FP64
)

# Count enabled modes
set(NEXT_MODE_COUNT 0)
foreach(m ${NEXT_MODES})
    if(${m})
        math(EXPR NEXT_MODE_COUNT "${NEXT_MODE_COUNT} + 1")
    endif()
endforeach()

# Enforce exactly one mode
if(NEXT_MODE_COUNT EQUAL 0)
    message(FATAL_ERROR "You must enable NEXT_FP64 or NEXT_FP32.")
elseif(NEXT_MODE_COUNT GREATER 1)
    message(FATAL_ERROR "Enable only one precision mode.")
endif()

# ============================
# Apply compile definitions
# ============================
if(NEXT_FP32)
    add_compile_definitions(NEXT_FP32)
elseif(NEXT_FP64)
    add_compile_definitions(NEXT_FP64)
endif()

# ============================
# Includes and sources
# ============================
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)
file(GLOB SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB ARGPARSE_FILES ${CMAKE_SOURCE_DIR}/argparse/*.cpp)

add_executable(next ${SRC_FILES} ${ARGPARSE_FILES})

find_package(OpenMP QUIET) 

if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP detected — enabling multithreading.")
    target_link_libraries(next PRIVATE OpenMP::OpenMP_CXX)
else()
    message(STATUS "OpenMP not found — building in single-threaded mode.")
endif()
