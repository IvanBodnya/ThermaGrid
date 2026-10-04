#pragma once

#include "QuadTreeNode.h"
#include <vector>

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
              int maxLevel = 3);
    
    ~AMRSolver();
    
    /**
     * @brief Run the simulation for a number of time steps
     * @param numSteps Number of time steps to run
     * @param dt Time step size
     * @param refineThreshold Gradient threshold for refinement
     */
    void run(int numSteps, double dt, double refineThreshold = 5.0);
    /**
     * @brief Advance the simulation by one time step
     * @param dt Time step size
     */
    void stepForward(double dt);
    
    /**
     * @brief Adapt the grid based on gradient magnitude
     * @param threshold Gradient threshold for refinement
     */
    void adaptGrid(double threshold);
    /**
     * @brief Set boundary temperature (Dirichlet boundary condition)
     * @param temp Fixed temperature on all boundaries
     */
    void setBoundaryTemperature(double temp);
    
    /**
     * @brief Get the current temperature field as a uniform grid
     * @param resolution Resolution of the output grid
     * @return 2D vector of temperatures
     */
    std::vector<std::vector<double>> getTemperatureField(int resolution = 100);
    
    /**
     * @brief Get the root node (for advanced access)
     */
    QuadTreeNode* getRoot() const;
    
    /**
     * @brief Get current simulation time
     */
    double getTime() const;
    
    /**
     * @brief Print the current grid structure
     */
    void printGrid() const;

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

    void setBoundaryCondition(
        QuadTreeNode::Direction direction,
        BoundaryType type,
        double value);

    void applyBoundaryConditions();

    void refineRoot(int levels = 1);
    
private:
    // At this point, region is a square, that has always 4 sides
    static constexpr int SIDES_COUNT = 4;

    QuadTreeNode* root;
    double domainSize;
    double alpha;
    int maxLevel;
    double time;
    double boundaryTemp;
    BoundaryCondition bc[SIDES_COUNT];
};