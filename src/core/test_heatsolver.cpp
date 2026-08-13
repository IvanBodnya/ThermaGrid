#include "AMRSolver.h"
#include <iostream>
#include <iomanip>

void printTemperatureField(const std::vector<std::vector<double>>& field) {
    std::cout << "Temperature Field:" << std::endl;
    std::cout << std::string(50, '-') << std::endl;

    // Print the full field
    for (int y = 0; y < field.size(); ++y) {
        for (int x = 0; x < field[y].size(); ++x) {
            std::cout << std::setw(6) << std::fixed << std::setprecision(1)
                      << field[y][x] << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  ThermaGrid - Heat Solver Test" << std::endl;
    std::cout << "========================================" << std::endl;

    // ========================================================================
    // Setup the solver
    // ========================================================================

    AMRSolver solver(1.0, 20.0, 0.01, 3);

    // ========================================================================
    // Set boundary conditions
    // ========================================================================

    // Option 1: All boundaries at 100°C
    solver.setBoundaryCondition(0, AMRSolver::DIRICHLET, 100.0);  // LEFT
    solver.setBoundaryCondition(1, AMRSolver::DIRICHLET, 100.0);  // RIGHT
    solver.setBoundaryCondition(2, AMRSolver::DIRICHLET, 100.0);  // TOP
    solver.setBoundaryCondition(3, AMRSolver::DIRICHLET, 100.0);  // BOTTOM

    // Option 2: Mixed boundaries (uncomment to try)
    // solver.setBoundaryCondition(0, AMRSolver::DIRICHLET, 100.0);  // LEFT: heated
    // solver.setBoundaryCondition(1, AMRSolver::DIRICHLET, 0.0);     // RIGHT: cold
    // solver.setBoundaryCondition(2, AMRSolver::DIRICHLET, 100.0);  // TOP: heated
    // solver.setBoundaryCondition(3, AMRSolver::NEUMANN, 0.0);      // BOTTOM: insulated

    // ========================================================================
    // Initialize with a heat source in the bottom-right
    // ========================================================================

    QuadTreeNode* root = solver.getRoot();

    // Refine to Level 2
    root->refine();
    root->children[0]->refine();
    root->children[1]->refine();
    root->children[2]->refine();
    root->children[3]->refine();
    root->buildNeighborsUsingGrid();

    // Set temperatures
    std::vector<QuadTreeNode*> leaves;
    root->getAllLeaves(leaves);

    int hotCells = 0;
    for (auto* leaf : leaves) {
        double cellSize = 1.0 / (1 << leaf->level);
        double cx = (leaf->gridX + 0.5) * cellSize;
        double cy = (leaf->gridY + 0.5) * cellSize;

        // Heat source in bottom-right corner (x > 0.5, y > 0.5)
        if (cx > 0.5 && cy > 0.5) {
            leaf->temperature = 300.0;
            hotCells++;
        } else {
            leaf->temperature = 20.0;
        }
    }

    std::cout << "Hot cells: " << hotCells << std::endl;

    // Rebuild neighbors
    root->buildNeighborsUsingGrid();

    // ========================================================================
    // Print initial state
    // ========================================================================

    std::cout << "\n=== Initial State ===" << std::endl;
    auto field = solver.getTemperatureField(20);
    printTemperatureField(field);

    // ========================================================================
    // Run the simulation
    // ========================================================================

    std::cout << "\nStarting simulation..." << std::endl;
    std::cout << "Initial grid: " << root->countNodes() << " nodes" << std::endl;

    solver.run(100, 0.001, 5.0);

    // ========================================================================
    // Print final state
    // ========================================================================

    std::cout << "\n=== Final State (after 100 steps) ===" << std::endl;
    field = solver.getTemperatureField(20);
    printTemperatureField(field);

    std::cout << "\n=== Simulation Complete ===" << std::endl;
    std::cout << "Final time: " << solver.getTime() << " seconds" << std::endl;
    std::cout << "Final nodes: " << root->countNodes() << std::endl;

    // ========================================================================
    // Run to equilibrium
    // ========================================================================

    std::cout << "\nRunning to equilibrium (1000 more steps)..." << std::endl;
    solver.run(1000, 0.001, 5.0);

    std::cout << "\n=== Equilibrium State ===" << std::endl;
    field = solver.getTemperatureField(20);
    printTemperatureField(field);

    std::cout << "\nFinal time: " << solver.getTime() << " seconds" << std::endl;
    std::cout << "Final nodes: " << root->countNodes() << std::endl;

    std::cout << "\n[OK] Heat solver test complete!" << std::endl;

    return 0;
}