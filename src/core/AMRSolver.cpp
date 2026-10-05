#include "QuadTreeNode.h"
#include "AMRSolver.h"
#include <vector>
#include <iostream>

/**
 * @brief Construct a new AMRSolver
 * @param domainSize Physical size of the domain (square)
 * @param initialTemp Initial temperature of the domain
 * @param alpha Thermal diffusivity (material property)
 * @param maxLevel Maximum refinement level allowed
 */
AMRSolver::AMRSolver(const double domainSize,
                     const double initialTemp,
                     const double alpha,
                     const int maxLevel)
    : domainSize(domainSize)
      , alpha(alpha)
      , maxLevel(maxLevel)
      , time(0.0) {
    // Create the root node
    root = new QuadTreeNode(initialTemp, 0, 0, 0);

    // Set a default boundary condition (Dirichlet, fixed temperature)
    boundaryTemp = 20.0;
}

AMRSolver::~AMRSolver() {
    delete root;
}

/**
 * @brief Run the simulation for a number of time steps
 * @param numSteps Number of time steps to run
 * @param dt Time step size
 * @param refineThreshold Gradient threshold for refinement
 */
void AMRSolver::run(const int numSteps, const double dt, const double refineThreshold) {
    for (int step = 0; step < numSteps; ++step) {
        // 1. Advance temperature by one time step
        stepForward(dt);

        // 2. Optionally refine the grid based on gradients
        if (refineThreshold > 0.0) {
            adaptGrid(refineThreshold);
        }

        // 3. Update time
        time += dt;

        // 4. Print progress (every 100 steps)
        if (step % 100 == 0) {
            std::cout << "Step " << step
                    << ", Time: " << time
                    << ", Cells: " << root->countNodes()
                    << std::endl;
        }
    }
}

/**
 * @brief Advance the simulation by one time step
 * @param dt Time step size
 */
void AMRSolver::stepForward(const double dt) {
    std::vector<QuadTreeNode *> leaves;
    root->getAllLeaves(leaves);

    std::vector<double> newTemps(leaves.size());

    for (size_t i = 0; i < leaves.size(); ++i) {
        QuadTreeNode *cell = leaves[i];

        // Compute the Laplacian
        double laplacian = cell->computeLaplacian();

        // Apply the heat equation
        double newTemp = cell->temperature + alpha * laplacian * dt;

        // Clamp temperatures to a reasonable range
        // This prevents 0°C artifacts from propagating
        if (newTemp < 0.0) newTemp = 0.0;
        if (newTemp > 500.0) newTemp = 500.0;

        newTemps[i] = newTemp;
    }

    // Update all cells
    for (size_t i = 0; i < leaves.size(); ++i) {
        leaves[i]->temperature = newTemps[i];
    }

    applyBoundaryConditions();

    time += dt;
}

/**
 * @brief Adapt the grid based on gradient magnitude
 * @param threshold Gradient threshold for refinement
 */
void AMRSolver::adaptGrid(const double threshold) {
    // Collect all leaf cells
    std::vector<QuadTreeNode *> leaves;
    root->getAllLeaves(leaves);

    // Refine cells with high gradients
    bool refined = false;
    for (auto *leaf: leaves) {
        if (leaf->level < maxLevel) {
            double grad = leaf->computeGradientMagnitude();
            if (grad > threshold) {
                leaf->refine();
                refined = true;
            }
        }
    }

    // If we refined anything, rebuild neighbors
    if (refined) {
        root->buildNeighborsUsingGrid();
        std::cout << "  Refined grid. New cells: " << root->countNodes() << std::endl;
    }
}

/**
 * @brief Set boundary temperature (Dirichlet boundary condition)
 * @param temp Fixed temperature on all boundaries
 */
void AMRSolver::setBoundaryTemperature(const double temp) {
    boundaryTemp = temp;
}

/**
 * @brief Get the current temperature field as a uniform grid
 * @param resolution Resolution of the output grid
 * @return 2D vector of temperatures
 */
std::vector<std::vector<double> > AMRSolver::getTemperatureField(const int resolution) {
    std::vector<double> buffer(resolution * resolution, 0.0);

    std::vector<QuadTreeNode *> leaves;
    root->getAllLeaves(leaves);

    int maxLevel = 0;
    for (auto *leaf: leaves) {
        if (leaf->level > maxLevel) maxLevel = leaf->level;
    }

    root->fillUniformGrid(buffer, resolution, resolution, root->getMaxLevel());

    std::vector<std::vector<double> > field(resolution, std::vector<double>(resolution, 0.0));
    for (int y = 0; y < resolution; ++y)
        for (int x = 0; x < resolution; ++x)
            field[y][x] = buffer[y * resolution + x];

    return field;
}

/**
 * @brief Get the root node (for advanced access)
 */
QuadTreeNode *AMRSolver::getRoot() const { return root; }

/**
 * @brief Get current simulation time
 */
double AMRSolver::getTime() const { return time; }

/**
 * @brief Print the current grid structure
 */
void AMRSolver::printGrid() const {
    std::cout << "=== Grid Structure ===" << std::endl;
    std::cout << "Total nodes: " << root->countNodes() << std::endl;
    std::cout << "Time: " << time << std::endl;

    std::vector<QuadTreeNode *> leaves;
    root->getAllLeaves(leaves);
    std::cout << "Leaf cells: " << leaves.size() << std::endl;

    std::cout << std::endl;
    root->printTree();
}

void AMRSolver::setBoundaryCondition(
    const QuadTreeNode::Direction direction,
    const BoundaryType type,
    const double value) {
    bc[direction] = {type, value};
}

void AMRSolver::applyBoundaryConditions() {
    std::vector<QuadTreeNode *> leaves;
    root->getAllLeaves(leaves);

    for (auto *leaf: leaves) {
        // Check which boundaries the cell is on
        for (int d = 0; d < SIDES_COUNT; ++d) {
            if (leaf->neighbors[d].empty()) {
                // Cell is on boundary side d
                const auto &condition = bc[d];

                switch (condition.type) {
                    case DIRICHLET:
                        leaf->temperature = condition.value;
                        break;

                    case NEUMANN:
                        // Neumann: fixed flux — don't modify temperature here.
                        // The flux is enforced via the gradient computation
                        // (which we'll fix separately).
                        break;

                    case ROBIN:
                        // Robin: mixed. Skip for now, or implement later.
                        break;
                }
            }
        }
    }
}

void AMRSolver::refineRoot(const int levels) {
    for (int i = 0; i < levels; ++i) {
        std::vector<QuadTreeNode *> leaves;
        root->getAllLeaves(leaves);
        for (auto *leaf: leaves) {
            if (leaf->level < maxLevel) {
                leaf->refine();
            }
        }
    }
    root->buildNeighborsUsingGrid();
}
