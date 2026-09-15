# Changelog

## [Unreleased]

### Added
- AMR grid overlay visualization (planned)
- Parameter sliders for dt, refinement threshold, and alpha (planned)
- Colorbar legend with absolute temperature scale (planned)
- Export of temperature field to PNG and CSV (planned)
- Documentation site (planned)

### Fixed
- `fillUniformGrid` renders only the top-left quadrant after grid refinement
- `Cells` counter shows 0 on startup instead of the actual node count
- Boundary cells retain 0°C values after refinement (flat grid fill bug)

## [0.1.0] - 2026-09-15

### Added
- **Core C++ engine**
    - Quad-tree data structure with `refine()` and `coarsen()` operations
    - Adaptive mesh refinement (AMR) with mixed-level neighbors
    - Full neighbor management supporting multiple neighbors per direction
    - Gradient computation (`dT/dx`, `dT/dy`, `|grad T|`)
    - Laplacian computation for the heat equation
    - Transient heat solver using explicit Euler time-stepping
    - Dirichlet boundary conditions (fixed temperature)
    - Gradient-based adaptive refinement

- **C API (`exports.h` / `exports.cpp`)**
    - `tg_create_solver` / `tg_destroy_solver` — solver lifecycle
    - `tg_step_forward` — advance one time step
    - `tg_adapt_grid` — refine grid based on gradients
    - `tg_run` — run N steps in one call
    - `tg_set_boundary_condition` — configure BCs
    - `tg_set_region_temperature` — initialize heat sources
    - `tg_get_temperature_field` — read the current field
    - `tg_get_node_count` / `tg_get_time` — read simulation state

- **C# WPF UI (`ThermaGrid.UI`)**
    - Cross-language interop via P/Invoke
    - Real-time heatmap rendering using `WriteableBitmap`
    - Start / Stop / Reset controls
    - Info panel showing time, cell count, and temperature range
    - Blue-to-red color gradient for temperature visualization

- **Build system**
    - CMake configuration for cross-platform builds
    - Shared library output (`ThermaGrid_Core.dll` on Windows)
    - Test executables for neighbors, gradients, and heat solver

- **Documentation**
    - Initial `README.md` with project overview
    - `CHANGELOG.md` (this file)

### Known Issues
- `fillUniformGrid` only fills the top-left quadrant after refinement — likely a coordinate-system mismatch between `level` and buffer resolution
- Boundary cells sometimes retain 0°C values due to empty spots in the flat grid
- `Cells: 0` is displayed on startup until the first simulation step
- The heatmap uses a relative color scale (min/max of the current field) instead of an absolute one

### Notes
- This is the **first MVP release**. The core pipeline (C++ solver → C API → C# UI) is working end-to-end. Refinement, physics, and visualization all function. Known bugs are visual artifacts, not fundamental architecture issues.

[Unreleased]: https://github.com/yourusername/ThermaGrid/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/yourusername/ThermaGrid/releases/tag/v0.1.0