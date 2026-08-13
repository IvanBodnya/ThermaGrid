cmake_minimum_required(VERSION 3.15)
project(ThermaGrid VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------------------------------------------------------------------
# Core library
# ----------------------------------------------------------------------------

add_library(ThermaGrid_Core
    src/core/QuadTreeNode.cpp
    src/core/test_neighbors.cpp
    src/core/test_neighbors.h
    src/core/AMRSolver.h
)

target_include_directories(ThermaGrid_Core
    PUBLIC
        src/core
)

# ----------------------------------------------------------------------------
# Main executable (for testing)
# ----------------------------------------------------------------------------

add_executable(ThermaGrid_Main
    src/core/main.cpp
)

target_link_libraries(ThermaGrid_Main
    ThermaGrid_Core
)

# Neighbor test executable
add_executable(ThermaGrid_Neighbors
        src/core/test_neighbors.cpp
)
target_link_libraries(ThermaGrid_Neighbors ThermaGrid_Core)

# Gradient test
add_executable(ThermaGrid_Gradients
        src/core/test_gradients.cpp
)
target_link_libraries(ThermaGrid_Gradients ThermaGrid_Core)

# Heat solver test
add_executable(ThermaGrid_HeatSolver
        src/core/test_heatsolver.cpp
)
target_link_libraries(ThermaGrid_HeatSolver ThermaGrid_Core)