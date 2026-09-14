#pragma once

// ============================================================================
// ThermaGrid C API
// 
// This header exposes the ThermaGrid simulation engine as a plain C API
// so it can be called from C# (or Python, or any other language) via P/Invoke.
//
// All functions use C linkage (extern "C") and C-compatible types only:
//   - No C++ classes
//   - No std::vector, std::string, etc.
//   - No references (&) — use pointers (*) instead
// ============================================================================

#ifdef _WIN32
    #ifdef THERMAGRID_EXPORTS
        #define TG_API __declspec(dllexport)
    #else
        #define TG_API __declspec(dllimport)
    #endif
#else
    #define TG_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------
// Solver lifecycle
// ---------------------------------------------------------------------------

/**
 * Create a new solver instance.
 * @param domainSize   Physical size of the domain (square), e.g. 1.0
 * @param initialTemp  Initial temperature everywhere, e.g. 20.0
 * @param alpha        Thermal diffusivity, e.g. 0.01
 * @param maxLevel     Maximum refinement level, e.g. 3
 * @return Opaque handle to the solver (or nullptr on failure)
 */
TG_API void* tg_create_solver(double domainSize,
                              double initialTemp,
                              double alpha,
                              int maxLevel);

/**
 * Destroy a solver instance and free all resources.
 */
TG_API void tg_destroy_solver(void* solver);

/**
 * Reset the solver to its initial state.
 */
TG_API void tg_reset_solver(void* solver);

// ---------------------------------------------------------------------------
// Simulation control
// ---------------------------------------------------------------------------

/**
 * Advance the simulation by one time step.
 */
TG_API void tg_step_forward(void* solver, double dt);

/**
 * Adapt the grid based on the current gradient field.
 * @param threshold  Gradient magnitude above which cells are refined
 */
TG_API void tg_adapt_grid(void* solver, double threshold);

/**
 * Run N time steps in one call (with optional refinement).
 */
TG_API void tg_run(void* solver,
                   int numSteps,
                   double dt,
                   double refineThreshold);

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

/**
 * Set a boundary condition.
 * @param side  0=LEFT, 1=RIGHT, 2=TOP, 3=BOTTOM
 * @param type  0=DIRICHLET, 1=NEUMANN, 2=ROBIN
 * @param value Temperature (Dirichlet) or flux (Neumann)
 */
TG_API void tg_set_boundary_condition(void* solver,
                                      int side,
                                      int type,
                                      double value);

/**
 * Set the initial temperature for a rectangular region.
 * Used to initialize heat sources.
 */
TG_API void tg_set_region_temperature(void* solver,
                                      double x0, double y0,
                                      double x1, double y1,
                                      double temperature);

// ---------------------------------------------------------------------------
// Data access (for visualization)
// ---------------------------------------------------------------------------

/**
 * Fill a caller-provided buffer with the temperature field,
 * sampled at a uniform resolution.
 * @param buffer       Pointer to a double array of size resolution*resolution
 * @param resolution   Number of samples per axis
 */
TG_API void tg_get_temperature_field(void* solver,
                                     double* buffer,
                                     int resolution);

/**
 * Get the number of leaf cells in the current grid.
 */
TG_API int tg_get_node_count(void* solver);

/**
 * Get the current simulation time.
 */
TG_API double tg_get_time(void* solver);

#ifdef __cplusplus
}
#endif