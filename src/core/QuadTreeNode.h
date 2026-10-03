#pragma once

#include <vector>

class QuadTreeNode {
public:
    // Neighbor directions
    enum Direction { TOP = 0, RIGHT = 1, BOTTOM = 2, LEFT = 3 };

    static constexpr int NUM_CHILDREN = 4;
    static constexpr int NUM_NEIGHBORS = 4;

    double temperature;
    int level;
    int gridX;
    int gridY;
    bool isLeaf;

    QuadTreeNode* children[NUM_CHILDREN]{};
    std::vector<QuadTreeNode*> neighbors[NUM_NEIGHBORS];

    explicit QuadTreeNode(double temp = 20.0, int lvl = 0, int x = 0, int y = 0);

    ~QuadTreeNode();

    QuadTreeNode(const QuadTreeNode&) = delete;
    QuadTreeNode& operator=(const QuadTreeNode&) = delete;

    QuadTreeNode(QuadTreeNode&& other) noexcept;
    QuadTreeNode& operator=(QuadTreeNode&& other) noexcept;

    // ========================================================================
    // Tree Queries
    // ========================================================================

    // creates children for a node and marks original one with isLeaf = false
    void refine();

    // deletes all children of a node and returns average temperature among the nodes
    double coarsen();

    // recursively counts all the nodes
    int countNodes() const;

    void getAllLeaves(std::vector<QuadTreeNode*>& leaves);

    // ========================================================================
    // Visualization
    // ========================================================================

    void printTree(int indent = 0) const;
    void printNeighbors(int indent = 0) const;
    void fillUniformGrid(
        std::vector<double>& buffer,
        int width,
        int height,
        int maxLevel) const;
    int getMaxLevel() const;

    // ========================================================================
    // Neighbor Management
    // ========================================================================

    void clearNeighbors();
    void buildNeighborsUsingGrid();

    // ========================================================================
    // Gradient Computation
    // ========================================================================

    /**
     * Compute the temperature gradient in the X direction (∂T/∂x)
     * Uses the LEFT and RIGHT neighbors
     * Returns 0.0 if no neighbors available
     */
    double computeGradientX() const ;

    /**
     * Compute the temperature gradient in the Y direction (∂T/∂y)
     */
    double computeGradientY() const;

    /**
     * Compute the gradient magnitude: |∇T| = sqrt((∂T/∂x)² + (∂T/∂y)²)
     * This is the key metric for deciding where to refine
     */
    double computeGradientMagnitude() const;

    /**
     * Compute the Laplacian: ∇²T = ∂²T/∂x² + ∂²T/∂y²
     * This is used in the heat equation: ∂T/∂t = α∇²T
     */
    double computeLaplacian() const ;
private:
    // Child indices
    enum ChildIndex { NE = 0, SE = 1, SW = 2, NW = 3};

    void addNeighbor(Direction dir, QuadTreeNode* neighbor);
};