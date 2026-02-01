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
# Precision / SIMD mode options
# ============================
option(NEXT_FP32        "Use 32-bit floats (scalar)"        OFF)
option(NEXT_FP64        "Use 64-bit floats (scalar)"        ON)
option(NEXT_SIMD32      "Use 32-bit SIMD (SSE)"             OFF)
option(NEXT_SIMD64      "Use 64-bit SIMD (SSE)"             OFF)
option(NEXT_AVX512_32   "Use 32-bit SIMD (AVX-512)"         OFF)
option(NEXT_AVX512_64   "Use 64-bit SIMD (AVX-512)"         OFF)

set(NEXT_MODES
    NEXT_FP32
    NEXT_FP64
    NEXT_SIMD32
    NEXT_SIMD64
    NEXT_AVX512_32
    NEXT_AVX512_64
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
    message(FATAL_ERROR "You must enable one precision/SIMD mode.")
elseif(NEXT_MODE_COUNT GREATER 1)
    message(FATAL_ERROR "You must enable ONLY ONE of: NEXT_FP32, NEXT_FP64, NEXT_SIMD32, NEXT_SIMD64, NEXT_AVX512_32, NEXT_AVX512_64.")
endif()

# ============================
# Apply compile definitions and flags
# ============================
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

# ============================
# Includes and sources
# ============================
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)
file(GLOB SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(next ${SRC_FILES})

# ============================
# OpenMP detection + CI-safe auto-install
# ============================
find_package(OpenMP QUIET)

if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP detected — enabling multithreading.")
    target_link_libraries(next PRIVATE OpenMP::OpenMP_CXX)

else()
    message(WARNING "OpenMP not found — attempting automatic installation (non-interactive).")

    # Default to failure unless we explicitly succeed
    set(OMP_INSTALL_RESULT 1)

    if(WIN32)
        message(STATUS "Installing OpenMP on Windows via winget (Build Tools)...")
        execute_process(
            COMMAND powershell -Command "winget install Microsoft.VisualStudio.2022.BuildTools --silent"
            RESULT_VARIABLE OMP_INSTALL_RESULT
        )

    elseif(UNIX AND NOT APPLE)
        message(STATUS "Installing OpenMP on Linux (non-interactive)...")

        if(EXISTS "/usr/bin/apt")
            execute_process(COMMAND sudo apt update -y)
            execute_process(
                COMMAND sudo apt install -y libomp-dev
                RESULT_VARIABLE OMP_INSTALL_RESULT
            )

        elseif(EXISTS "/usr/bin/dnf")
            execute_process(
                COMMAND sudo dnf install -y libomp-devel
                RESULT_VARIABLE OMP_INSTALL_RESULT
            )

        elseif(EXISTS "/usr/bin/pacman")
            execute_process(
                COMMAND sudo pacman -Sy --noconfirm openmp
                RESULT_VARIABLE OMP_INSTALL_RESULT
            )

        elseif(EXISTS "/usr/bin/zypper")
            execute_process(
                COMMAND sudo zypper install -y libomp-devel
                RESULT_VARIABLE OMP_INSTALL_RESULT
            )

        else()
            message(FATAL_ERROR "Unsupported Linux distro for automatic OpenMP installation.")
        endif()

    else()
        message(FATAL_ERROR "Automatic OpenMP installation not supported on this OS.")
    endif()

    if(OMP_INSTALL_RESULT EQUAL 0)
        message(STATUS "OpenMP installed successfully. Re-run CMake configuration.")
        message(FATAL_ERROR "")
    else()
        message(FATAL_ERROR "Automatic OpenMP installation failed. Check CI logs and package manager output.")
    endif()

endif()