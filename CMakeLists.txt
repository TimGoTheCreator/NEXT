cmake_minimum_required(VERSION 3.16)

project(NEXT
    VERSION 2026.08.12
    DESCRIPTION "Newtonian EXact Trajectories: Multi-Threaded & Distributed N-Body Solver"
    LANGUAGES C CXX
)

# ==============================================================================
# C++ Standard Requirements
# ==============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==============================================================================
# Build Options & HPC Features
# ==============================================================================
option(NEXT_FP32 "Use 32-bit floating point precision (FP32)" OFF)
option(NEXT_FP64 "Use 64-bit floating point precision (FP64)" ON)
option(NEXT_MPI "Enable MPI distributed memory parallelism" OFF)
option(NEXT_NATIVE_ARCH "Enable architecture-native host CPU optimizations (-march=native)" OFF)
option(NEXT_ENABLE_LTO "Enable Link-Time Optimization / Interprocedural Optimization" OFF)
option(ENABLE_VEC_REPORT "Enable compiler vectorization reports" OFF)
option(NEXT_ENABLE_CUDA "Enable NVIDIA CUDA GPU Acceleration" OFF)
option(NEXT_BENCHMARK "Enable step benchmark timing and logging to log.txt" OFF)

# ==============================================================================
# Precision Mode Validation
# ==============================================================================
set(NEXT_MODE_COUNT 0)
if(NEXT_FP32)
    math(EXPR NEXT_MODE_COUNT "${NEXT_MODE_COUNT} + 1")
endif()
if(NEXT_FP64)
    math(EXPR NEXT_MODE_COUNT "${NEXT_MODE_COUNT} + 1")
endif()

if(NOT NEXT_MODE_COUNT EQUAL 1)
    message(FATAL_ERROR "[NEXT HPC] Exactly ONE precision mode must be selected (NEXT_FP32 or NEXT_FP64).")
endif()

# ==============================================================================
# Target & Source Configuration
# ==============================================================================
file(GLOB_RECURSE SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB ARGPARSE_FILES ${CMAKE_SOURCE_DIR}/argparse/*.cpp)

add_executable(next ${SRC_FILES} ${ARGPARSE_FILES})

target_include_directories(next PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/argparse
)

# ==============================================================================
# Precision & Feature Compile Definitions
# ==============================================================================
if(NEXT_FP32)
    target_compile_definitions(next PRIVATE NEXT_FP32)
    message(STATUS "[NEXT HPC] Precision: FP32 (Single Precision)")
elseif(NEXT_FP64)
    target_compile_definitions(next PRIVATE NEXT_FP64)
    message(STATUS "[NEXT HPC] Precision: FP64 (Double Precision)")
endif()

if(NEXT_BENCHMARK)
    target_compile_definitions(next PRIVATE NEXT_BENCHMARK)
    message(STATUS "[NEXT HPC] Benchmark logging enabled (NEXT_BENCHMARK)")
endif()

if(NEXT_ENABLE_CUDA)
    target_compile_definitions(next PRIVATE NEXT_ENABLE_CUDA)
    message(STATUS "[NEXT HPC] NVIDIA CUDA GPU Acceleration enabled (NEXT_ENABLE_CUDA)")
endif()

# ==============================================================================
# Compiler Optimization & Warning Flags
# ==============================================================================
if(MSVC)
    target_compile_options(next PRIVATE /W3 /O2)
else()
    target_compile_options(next PRIVATE -Wall -O3)

    if(NEXT_NATIVE_ARCH)
        target_compile_options(next PRIVATE -march=native)
        message(STATUS "[NEXT HPC] Host CPU tuning enabled (-march=native)")
    endif()
endif()

# ==============================================================================
# Link-Time Optimization (LTO / IPO)
# ==============================================================================
if(NEXT_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
    if(ipo_supported)
        set_property(TARGET next PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
        message(STATUS "[NEXT HPC] Interprocedural Optimization (LTO/IPO) enabled")
    else()
        message(WARNING "[NEXT HPC] IPO/LTO requested but not supported: ${ipo_error}")
    endif()
endif()

# ==============================================================================
# Compiler Vectorization Reports
# ==============================================================================
if(ENABLE_VEC_REPORT)
    if(CMAKE_CXX_COMPILER_ID MATCHES "IntelLLVM")
        target_compile_options(next PRIVATE -Rpass=loop-vectorize -Rpass-missed=loop-vectorize)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(next PRIVATE -fopt-info-vec-optimized -fopt-info-vec-missed)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(next PRIVATE -Rpass=loop-vectorize -Rpass-missed=loop-vectorize)
    endif()
    message(STATUS "[NEXT HPC] Compiler vectorization reporting enabled")
endif()

# ==============================================================================
# OpenMP Shared-Memory Multi-Threading
# ==============================================================================
find_package(OpenMP QUIET)
if(OpenMP_CXX_FOUND)
    message(STATUS "[NEXT HPC] OpenMP detected - enabling multi-threading")
    target_link_libraries(next PRIVATE OpenMP::OpenMP_CXX)
else()
    message(STATUS "[NEXT HPC] OpenMP not found - building single-threaded")
endif()

# ==============================================================================
# MPI Distributed Parallelism
# ==============================================================================
if(NEXT_MPI)
    find_package(MPI REQUIRED)
    message(STATUS "[NEXT HPC] MPI detected - enabling cluster distribution")
    target_compile_definitions(next PRIVATE NEXT_MPI)
    target_link_libraries(next PRIVATE MPI::MPI_CXX)
endif()

# ==============================================================================
# HDF5 I/O Library Detection
# ==============================================================================
if(MSVC AND DEFINED ENV{HDF5_DIR})
    set(HDF5_ROOT "$ENV{HDF5_DIR}")
    set(CMAKE_PREFIX_PATH "${HDF5_ROOT};${CMAKE_PREFIX_PATH}")
endif()

if(MINGW)
    if(DEFINED ENV{MSYSTEM_PREFIX})
        set(CMAKE_PREFIX_PATH "$ENV{MSYSTEM_PREFIX};${CMAKE_PREFIX_PATH}")
    endif()
    set(CMAKE_PREFIX_PATH "C:/tools/msys64/mingw64;C:/msys64/ucrt64;C:/msys64/mingw64;${CMAKE_PREFIX_PATH}")
endif()

find_package(HDF5 REQUIRED COMPONENTS C HL)

include_directories(${HDF5_INCLUDE_DIRS})
target_include_directories(next PRIVATE ${HDF5_INCLUDE_DIRS} ${HDF5_C_INCLUDE_DIRS})
target_link_libraries(next PRIVATE ${HDF5_LIBRARIES})

if(TARGET HDF5::HDF5)
    target_link_libraries(next PRIVATE HDF5::HDF5)
endif()
if(TARGET hdf5::hdf5)
    target_link_libraries(next PRIVATE hdf5::hdf5)
endif()

# ==============================================================================
# Post-Build Executable Copy
# ==============================================================================
if(NEXT_COPY_TO_CMAKE_SOURCE_DIR)
    add_custom_command(TARGET next POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE:next>
                ${CMAKE_SOURCE_DIR}/$<TARGET_FILE_NAME:next>
        COMMENT "[NEXT Build] Copying executable to source directory"
    )
endif()
