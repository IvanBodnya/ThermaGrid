#pragma once

#include "QuadTreeNode.h"
#include <vector>
#include <iostream>

/**
 * @brief AMR-based heat solver for the transient heat equation
 * 
 * Solves ∂T/∂t = α∇²T using explicit Euler time-stepping
 * with adaptive mesh refinement based on temperature gradients
 */
class AMRSolver {
public:
    /**
     * @brief Construct a new AMRSolver
     * @param domainSize Physical size of the domain (square)
     * @param initialTemp Initial temperature of the domain
     * @param alpha Thermal diffusivity (material property)
     * @param maxLevel Maximum refinement level allowed
     */
    AMRSolver(double domainSize = 1.0, 
              double initialTemp = 20.0, 
              double alpha = 0.01,
              int maxLevel = 3)
        : domainSize(domainSize)
        , alpha(alpha)
        , maxLevel(maxLevel)
        , time(0.0) {
        // Create the root node
        root = new QuadTreeNode(initialTemp, 0, 0, 0);
        
        // Set a default boundary condition (Dirichlet, fixed temperature)
        boundaryTemp = 20.0;
    }
    
    ~AMRSolver() {
        delete root;
    }
    
    /**
     * @brief Run the simulation for a number of time steps
     * @param numSteps Number of time steps to run
     * @param dt Time step size
     * @param refineThreshold Gradient threshold for refinement
     */
    void run(int numSteps, double dt, double refineThreshold = 5.0) {
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
    void stepForward(double dt) {
        // Collect all leaf cells
        std::vector<QuadTreeNode*> leaves;
        root->getAllLeaves(leaves);
        
        // Compute new temperatures
        std::vector<double> newTemps(leaves.size());
        for (size_t i = 0; i < leaves.size(); ++i) {
            QuadTreeNode* cell = leaves[i];
            
            // Compute the Laplacian
            double laplacian = cell->computeLaplacian();
            
            // Apply the heat equation (explicit Euler)
            newTemps[i] = cell->temperature + alpha * laplacian * dt;
        }
        
        // Update all cells with new temperatures
        for (size_t i = 0; i < leaves.size(); ++i) {
            leaves[i]->temperature = newTemps[i];
        }
        
        // Apply boundary conditions
        applyBoundaryConditions();
    }
    
    /**
     * @brief Adapt the grid based on gradient magnitude
     * @param threshold Gradient threshold for refinement
     */
    void adaptGrid(double threshold) {
        // Collect all leaf cells
        std::vector<QuadTreeNode*> leaves;
        root->getAllLeaves(leaves);
        
        // Refine cells with high gradients
        bool refined = false;
        for (auto* leaf : leaves) {
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
    void setBoundaryTemperature(double temp) {
        boundaryTemp = temp;
    }
    
    /**
     * @brief Get the current temperature field as a uniform grid
     * @param resolution Resolution of the output grid
     * @return 2D vector of temperatures
     */
    std::vector<std::vector<double>> getTemperatureField(int resolution = 100) const {
        // Create a buffer
        std::vector<double> buffer(resolution * resolution, 0.0);
        
        // Fill the buffer from the tree
        root->fillUniformGrid(buffer, resolution, resolution, 0, 0, resolution);
        
        // Convert to 2D vector
        std::vector<std::vector<double>> field(resolution, std::vector<double>(resolution, 0.0));
        for (int y = 0; y < resolution; ++y) {
            for (int x = 0; x < resolution; ++x) {
                field[y][x] = buffer[y * resolution + x];
            }
        }
        return field;
    }
    
    /**
     * @brief Get the root node (for advanced access)
     */
    QuadTreeNode* getRoot() const { return root; }
    
    /**
     * @brief Get current simulation time
     */
    double getTime() const { return time; }
    
    /**
     * @brief Print the current grid structure
     */
    void printGrid() const {
        std::cout << "=== Grid Structure ===" << std::endl;
        std::cout << "Total nodes: " << root->countNodes() << std::endl;
        std::cout << "Time: " << time << std::endl;
        
        std::vector<QuadTreeNode*> leaves;
        root->getAllLeaves(leaves);
        std::cout << "Leaf cells: " << leaves.size() << std::endl;
        
        std::cout << std::endl;
        root->printTree();
    }

    // Boundary condition types
    enum BoundaryType {
        DIRICHLET,  // Fixed temperature
        NEUMANN,    // Fixed flux (insulated)
        ROBIN       // Convection
    };

    struct BoundaryCondition {
        BoundaryType type;
        double value;  // Temperature for Dirichlet, flux for Neumann
    };

    void setBoundaryCondition(int side, BoundaryType type, double value) {
        // side: 0=left, 1=right, 2=top, 3=bottom
        bc[side] = {type, value};
    }

    void applyBoundaryConditions() {
        std::vector<QuadTreeNode*> leaves;
        root->getAllLeaves(leaves);

        for (auto* leaf : leaves) {
            // Check which boundaries the cell is on
            bool onBoundary[4] = {false, false, false, false};

            for (int d = 0; d < 4; ++d) {
                if (leaf->neighbors[d].empty()) {
                    onBoundary[d] = true;
                }
            }

            // Apply boundary conditions for each side
            if (onBoundary[0]) { // LEFT
                if (bc[0].type == DIRICHLET) {
                    leaf->temperature = bc[0].value;
                }
                // Neumann: no change (insulated)
            }

            if (onBoundary[1]) { // RIGHT
                if (bc[1].type == DIRICHLET) {
                    leaf->temperature = bc[1].value;
                }
                // Neumann: no change (insulated)
            }

            if (onBoundary[2]) { // TOP
                if (bc[2].type == DIRICHLET) {
                    leaf->temperature = bc[2].value;
                }
                // Neumann: no change (insulated)
            }

            if (onBoundary[3]) { // BOTTOM
                if (bc[3].type == DIRICHLET) {
                    leaf->temperature = bc[3].value;
                }
                // Neumann: no change (insulated)
            }
        }
    }
    
private:
    QuadTreeNode* root;
    double domainSize;
    double alpha;
    int maxLevel;
    double time;
    double boundaryTemp;
    BoundaryCondition bc[4];  // Boundary conditions for each side
};