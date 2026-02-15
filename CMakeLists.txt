cmake_minimum_required(VERSION 3.10)

project(NEXT)

# ============================
# Detect compiler environment
# ============================
if (MINGW)
    message(STATUS "Detected MinGW GCC — using MinGW settings")
    set(USING_MINGW TRUE)
elseif (MSVC)
    message(STATUS "Detected MSVC — using MSVC settings")
    set(USING_MSVC TRUE)
elseif (APPLE)
    message(STATUS "Detected macOS — limited MPI support")
    set(USING_APPLE TRUE)
elseif (UNIX)
    message(STATUS "Detected Linux/UNIX — using standard settings")
    set(USING_UNIX TRUE)
endif()

# ============================
# Options
# ============================
option(NEXT_FP32 "Use 32-bit floats (scalar)" OFF)
option(NEXT_FP64 "Use 64-bit floats (scalar)" ON)
option(NEXT_MPI "Enable MPI support" OFF)

option(NEXT_COPY_TO_CMAKE_SOURCE_DIR "Copy final executable from build dir to source dir" ON)

# ============================
# Precision modes
# ============================
set(NEXT_MODES NEXT_FP32 NEXT_FP64)

set(NEXT_MODE_COUNT 0)
foreach(m ${NEXT_MODES})
    if(${m})
        math(EXPR NEXT_MODE_COUNT "${NEXT_MODE_COUNT} + 1")
    endif()
endforeach()

if(NEXT_MODE_COUNT EQUAL 0)
    message(FATAL_ERROR "You must enable NEXT_FP64 or NEXT_FP32.")
elseif(NEXT_MODE_COUNT GREATER 1)
    message(FATAL_ERROR "Enable only one precision mode.")
endif()

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
file(GLOB_RECURSE SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB ARGPARSE_FILES ${CMAKE_SOURCE_DIR}/argparse/*.cpp)

add_executable(next ${SRC_FILES} ${ARGPARSE_FILES})

# ============================
# Vectorization reports
# ============================
option(ENABLE_VEC_REPORT "Enable compiler vectorization reports" OFF)

if(ENABLE_VEC_REPORT)
    if(CMAKE_CXX_COMPILER_ID MATCHES "IntelLLVM")
        target_compile_options(next PRIVATE -O3 -Rpass=loop-vectorize -Rpass-missed=loop-vectorize)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(next PRIVATE -O3 -fopt-info-vec-optimized -fopt-info-vec-missed)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(next PRIVATE -O3 -Rpass=loop-vectorize -Rpass-missed=loop-vectorize)
    endif()
endif()

# ============================
# OpenMP
# ============================
find_package(OpenMP QUIET)
if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP detected — enabling multithreading.")
    if(MSVC)
        target_compile_options(next PRIVATE /openmp:llvm)
    else()
        target_link_libraries(next PRIVATE OpenMP::OpenMP_CXX)
    endif()
else()
    message(STATUS "OpenMP not found — building in single-threaded mode.")
endif()

# ============================
# MPI detection
# ============================
if(NEXT_MPI)
    if(USING_UNIX)
        # Linux: standard OpenMPI
        find_package(MPI REQUIRED)
        message(STATUS "MPI (OpenMPI) detected — enabling distributed memory parallelism.")
        target_compile_definitions(next PRIVATE NEXT_MPI)
        target_link_libraries(next PRIVATE MPI::MPI_CXX)

    elseif(USING_MSVC OR USING_MINGW)
        # Windows: Microsoft MPI
        find_package(MPI REQUIRED)
        message(STATUS "MPI (MS-MPI) detected — enabling distributed memory parallelism.")
        target_compile_definitions(next PRIVATE NEXT_MPI)
        target_link_libraries(next PRIVATE MPI::MPI_CXX)

    elseif(USING_APPLE)
        message(WARNING "MPI requested, but macOS does not ship MPI by default. Please install OpenMPI via Homebrew (brew install open-mpi).")
        find_package(MPI REQUIRED)
        target_compile_definitions(next PRIVATE NEXT_MPI)
        target_link_libraries(next PRIVATE MPI::MPI_CXX)
    endif()
endif()

# ============================
# HDF5 detection
# ============================
if (USING_MSVC AND DEFINED ENV{HDF5_DIR})
    set(HDF5_ROOT "$ENV{HDF5_DIR}")
    set(CMAKE_PREFIX_PATH "${HDF5_ROOT};${CMAKE_PREFIX_PATH}")
endif()

if (USING_MINGW)
    set(CMAKE_PREFIX_PATH "C:/tools/msys64/mingw64")
    message(STATUS "Using MSYS2 MinGW HDF5 path: ${CMAKE_PREFIX_PATH}")
endif()

find_package(HDF5 REQUIRED COMPONENTS C HL)

include_directories(${HDF5_INCLUDE_DIRS})
target_link_libraries(next PRIVATE ${HDF5_LIBRARIES})

# ============================
# Optional: Copy executable to source dir
# ============================
if(NEXT_COPY_TO_CMAKE_SOURCE_DIR)
    add_custom_command(TARGET next POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE:next>
                ${CMAKE_SOURCE_DIR}/$<TARGET_FILE_NAME:next>
        COMMENT "Copying executable to CMake source directory"
    )
endif()
