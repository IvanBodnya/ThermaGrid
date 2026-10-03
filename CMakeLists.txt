cmake_minimum_required(VERSION 3.15)
project(ThermaGrid VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------------------------------------------------------------------
# Core sources (shared between static and shared builds)
# ----------------------------------------------------------------------------

set(CORE_SOURCES
        src/core/QuadTreeNode.cpp
        src/core/exports.cpp
)

set(CORE_HEADERS
        src/core/QuadTreeNode.h
        src/core/AMRSolver.h
        src/core/exports.h
)

# ----------------------------------------------------------------------------
# Static library (for C++ executables)
# ----------------------------------------------------------------------------

add_library(ThermaGrid_Core_Static STATIC ${CORE_SOURCES})

target_sources(ThermaGrid_Core_Static PRIVATE ${CORE_HEADERS})

target_include_directories(ThermaGrid_Core_Static
        PUBLIC
        src/core
)

# Tell exports.h we're building the static library (no dllimport/dlllexport)
target_compile_definitions(ThermaGrid_Core_Static PRIVATE THERMAGRID_STATIC)

# ----------------------------------------------------------------------------
# Shared library (for C# UI — exposes only the C API)
# ----------------------------------------------------------------------------

add_library(ThermaGrid_Core SHARED ${CORE_SOURCES})

target_sources(ThermaGrid_Core PRIVATE ${CORE_HEADERS})

target_include_directories(ThermaGrid_Core
        PUBLIC
        src/core
)

# Tell exports.h we're building the DLL (dllexport on Windows)
target_compile_definitions(ThermaGrid_Core PRIVATE THERMAGRID_EXPORTS)

# ----------------------------------------------------------------------------
# Main executable (for testing)
# ----------------------------------------------------------------------------

add_executable(ThermaGrid_Main
        src/core/main.cpp
)

target_link_libraries(ThermaGrid_Main PRIVATE ThermaGrid_Core_Static)

# ----------------------------------------------------------------------------
# Test executables
# ----------------------------------------------------------------------------

add_executable(ThermaGrid_Neighbors
        src/core/test_neighbors.cpp
)
target_link_libraries(ThermaGrid_Neighbors PRIVATE ThermaGrid_Core_Static)

add_executable(ThermaGrid_Gradients
        src/core/test_gradients.cpp
)
target_link_libraries(ThermaGrid_Gradients PRIVATE ThermaGrid_Core_Static)

add_executable(ThermaGrid_HeatSolver
        src/core/test_heatsolver.cpp
)
target_link_libraries(ThermaGrid_HeatSolver PRIVATE ThermaGrid_Core_Static)

# ----------------------------------------------------------------------------
# Output directories
# ----------------------------------------------------------------------------

set_target_properties(ThermaGrid_Core PROPERTIES
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

set_target_properties(ThermaGrid_Core_Static PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

foreach(target ThermaGrid_Main ThermaGrid_Neighbors ThermaGrid_Gradients ThermaGrid_HeatSolver)
    set_target_properties(${target} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endforeach()