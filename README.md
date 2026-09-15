# ThermaGrid
## Adaptive Mesh Refinement for Transient Heat Transfer Simulation

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)]()
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-blue)]()
[![License](https://img.shields.io/badge/license-MIT-green)]()

[![Version](https://img.shields.io/badge/version-0.1.0-blue.svg)](CHANGELOG.md)
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

ThermaGrid is a high-performance C++/C# simulation platform that solves the 2D transient heat equation using adaptive mesh refinement (AMR). It dynamically refines the computational grid only where temperature gradients are steep, achieving 3-5x speedup compared to uniform grids.

## 🚀 Key Features

- **Adaptive Mesh Refinement**: Quad-tree based grid refinement based on temperature gradients
- **Transient Heat Solver**: Solves ∂T/∂t = α∇²T + Q(x,y,t) with explicit and ADI methods
- **Real-time Visualization**: C# WPF frontend with interactive heatmap rendering
- **Cross-Language Architecture**: High-performance C++ kernel with C# UI
- **Renewable Energy Focus**: Optimized for battery thermal management and solar receiver analysis
- **Parallel Computing**: OpenMP support for multi-core performance
