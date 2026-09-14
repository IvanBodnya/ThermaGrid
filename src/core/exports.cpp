#include "exports.h"
#include "AMRSolver.h"
#include <vector>
#include <iostream>

// ============================================================================
// Solver lifecycle
// ============================================================================

void* tg_create_solver(double domainSize,
                       double initialTemp,
                       double alpha,
                       int maxLevel) {
    try {
        AMRSolver* solver = new AMRSolver(domainSize, initialTemp, alpha, maxLevel);
        solver->refineRoot(2);  // Refine to level 2 (16 cells)
        return static_cast<void*>(solver);
    } catch (const std::exception& e) {
        std::cerr << "tg_create_solver failed: " << e.what() << std::endl;
        return nullptr;
    }
}

void tg_destroy_solver(void* solver) {
    if (!solver) return;
    delete static_cast<AMRSolver*>(solver);
}

void tg_reset_solver(void* solver) {
    if (!solver) return;
    // TODO: implement reset in AMRSolver
}

// ============================================================================
// Simulation control
// ============================================================================

void tg_step_forward(void* solver, double dt) {
    if (!solver) return;
    static_cast<AMRSolver*>(solver)->stepForward(dt);
}

void tg_adapt_grid(void* solver, double threshold) {
    if (!solver) return;
    static_cast<AMRSolver*>(solver)->adaptGrid(threshold);
}

void tg_run(void* solver, int numSteps, double dt, double refineThreshold) {
    if (!solver) return;
    static_cast<AMRSolver*>(solver)->run(numSteps, dt, refineThreshold);
}

// ============================================================================
// Configuration
// ============================================================================

void tg_set_boundary_condition(void* solver, int side, int type, double value) {
    if (!solver) return;
    // TODO: wire this to AMRSolver once we have a proper BC API
    auto* s = static_cast<AMRSolver*>(solver);
    s->setBoundaryTemperature(value);
}

void tg_set_region_temperature(void* solver,
                               double x0, double y0,
                               double x1, double y1,
                               double temperature) {
    if (!solver) return;
    auto* s = static_cast<AMRSolver*>(solver);
    QuadTreeNode* root = s->getRoot();
    
    std::vector<QuadTreeNode*> leaves;
    root->getAllLeaves(leaves);
    
    for (auto* leaf : leaves) {
        double cellSize = 1.0 / (1 << leaf->level);
        double cx = (leaf->gridX + 0.5) * cellSize;
        double cy = (leaf->gridY + 0.5) * cellSize;
        
        if (cx >= x0 && cx <= x1 && cy >= y0 && cy <= y1) {
            leaf->temperature = temperature;
        }
    }
}

// ============================================================================
// Data access
// ============================================================================

void tg_get_temperature_field(void* solver, double* buffer, int resolution) {
    if (!solver || !buffer) return;
    
    auto* s = static_cast<AMRSolver*>(solver);
    QuadTreeNode* root = s->getRoot();

    // Find max level
    std::vector<QuadTreeNode*> leaves;
    root->getAllLeaves(leaves);
    int maxLevel = 0;
    for (auto* leaf : leaves) {
        if (leaf->level > maxLevel) maxLevel = leaf->level;
    }

    // Create a temporary buffer
    std::vector<double> tmp(resolution * resolution, 0.0);

    // Fill using the correct maxLevel
    root->fillUniformGrid(tmp, resolution, resolution, maxLevel);
    
    // Copy to caller's buffer
    for (int i = 0; i < resolution * resolution; ++i) {
        buffer[i] = tmp[i];
    }
}

int tg_get_node_count(void* solver) {
    if (!solver) return 0;
    return static_cast<AMRSolver*>(solver)->getRoot()->countNodes();
}

double tg_get_time(void* solver) {
    if (!solver) return 0.0;
    return static_cast<AMRSolver*>(solver)->getTime();
}