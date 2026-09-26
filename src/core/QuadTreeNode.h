#pragma once

#include <vector>
#include <iostream>
#include <memory>
#include <cmath>

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

    explicit QuadTreeNode(const double temp = 20.0, const int lvl = 0, const int x = 0, const int y = 0)
        : temperature(temp), level(lvl), gridX(x), gridY(y), isLeaf(true) {

        for (int i = 0; i < NUM_CHILDREN; ++i) {
            children[i] = nullptr;
        }
    }

    ~QuadTreeNode() {
        for (int i = 0; i < NUM_CHILDREN; ++i) {
            delete children[i];
            children[i] = nullptr;
        }
    }

    QuadTreeNode(const QuadTreeNode&) = delete;
    QuadTreeNode& operator=(const QuadTreeNode&) = delete;

    QuadTreeNode(QuadTreeNode&& other) noexcept
         : temperature(other.temperature)
         , level(other.level)
         , gridX(other.gridX)
         , gridY(other.gridY)
         , isLeaf(other.isLeaf) {
        for (int i = 0; i < NUM_CHILDREN; ++i) {
            children[i] = other.children[i];
            other.children[i] = nullptr;
        }

        for (int i = 0; i < NUM_NEIGHBORS; ++i) {
            neighbors[i] = std::move(other.neighbors[i]);
        }
    }

    QuadTreeNode& operator=(QuadTreeNode&& other) noexcept {
        if (this != &other) {
            for (int i = 0; i < NUM_CHILDREN; ++i) {
                delete children[i];
                children[i] = nullptr;
            }

            temperature = other.temperature;
            isLeaf = other.isLeaf;
            level = other.level;
            gridX = other.gridX;
            gridY = other.gridY;

            for (int i = 0; i < NUM_CHILDREN; ++i) {
                children[i] = other.children[i];
                other.children[i] = nullptr;
            }

            for (int i = 0; i < NUM_NEIGHBORS; ++i) {
                neighbors[i] = std::move(other.neighbors[i]);
            }
        }
        return *this;
    }

    // ========================================================================
    // Tree Queries
    // ========================================================================

    // creates children for a node and marks original one with isLeaf = false
    void refine() {
        if (!isLeaf) return;

        // Validate that current coordinates are valid for this level
        // TODO: verify logic
        int maxCoord = (1 << level) - 1;
        if (gridX < 0 || gridX > maxCoord || gridY < 0 || gridY > maxCoord) {
            throw std::runtime_error("Invalid grid coordinates during refinement!");
        }

        // Child 0 (NW)
        // Child 1 (NE)
        // Child 2 (SE)
        // Child 3 (SW)
        children[0] = new QuadTreeNode(temperature, level + 1, gridX * 2, gridY * 2);
        children[1] = new QuadTreeNode(temperature, level + 1, gridX * 2 + 1, gridY * 2);
        children[2] = new QuadTreeNode(temperature, level + 1, gridX * 2 + 1, gridY * 2 + 1);
        children[3] = new QuadTreeNode(temperature, level + 1, gridX * 2, gridY * 2 + 1);

        isLeaf = false;
    }

    // deletes all children of a node and returns average temperature among the nodes
    double coarsen() {
        if (isLeaf) return temperature;

        double sum = 0.0;
        for (int i = 0; i < NUM_CHILDREN; ++i) {
            if (children[i]) {
                sum += children[i]->temperature;
            }
        }
        temperature = sum / NUM_CHILDREN;

        for (int i = 0; i < NUM_CHILDREN; ++i) {
            // delete children recursively
            children[i]->coarsen();

            delete children[i];
            children[i] = nullptr;
        }
        isLeaf = true;
        return temperature;
    }

    // recursively counts all the nodes
    int countNodes() const {
        if (isLeaf) return 1;
        int count = 1;
        for (int i = 0; i < NUM_CHILDREN; ++i) {
            if (children[i]) {
                count += children[i]->countNodes();
            }
        }

        return count;
    }

    void getAllLeaves(std::vector<QuadTreeNode*>& leaves) {
        if (isLeaf) {
            leaves.push_back(this);
        } else {
            for (int i = 0; i < NUM_CHILDREN; ++i) {
                if (children[i]) {
                    children[i]->getAllLeaves(leaves);
                }
            }
        }
    }

    // ========================================================================
    // Visualization
    // ========================================================================

    void printTree(const int indent = 0) const {
        std::cout << std::string(indent, ' ')
                  << "Level " << level
                  << ", Pos(" << gridX << "," << gridY << ")"
                  << ", Temp: " << temperature
                  << ", Leaf: " << (isLeaf ? "Yes" : "No")
                  << ", Nodes: " << countNodes() << std::endl;

        if (!isLeaf) {
            for (int i = 0; i < NUM_CHILDREN; ++i) {
                if (children[i]) {
                    children[i]->printTree(indent + 2);
                }
            }
        }
    }

    void printNeighbors(int indent = 0) const {
        std::string prefix(indent, ' ');
        const char* dirNames[] = {"TOP", "RIGHT", "BOTTOM", "LEFT"};

        std::cout << prefix << "Node (" << gridX << "," << gridY << ") Lv" << level << std::endl;
        for (int d = 0; d < NUM_NEIGHBORS; ++d) {
            if (!neighbors[d].empty()) {
                std::cout << prefix << "  " << dirNames[d] << ": ";
                for (const auto* neighbor : neighbors[d]) {
                    std::cout << "(" << neighbor->gridX << "," << neighbor->gridY
                              << ") Lv" << neighbor->level << " ";
                }
                std::cout << std::endl;
            }
        }
    }

    void fillUniformGrid(
        std::vector<double>& buffer,
        int width,
        int height,
        int maxLevel) const {
        // Physical size of this cell (domain is [0,1] x [0,1])
        double cellSize = 1.0 / (1 << level);

        // Physical position of top-left corner
        double px0 = gridX * cellSize;
        double py0 = gridY * cellSize;

        // Map to buffer pixels
        int bx0 = static_cast<int>(px0 * width);
        int by0 = static_cast<int>(py0 * height);
        int bx1 = static_cast<int>((px0 + cellSize) * width);
        int by1 = static_cast<int>((py0 + cellSize) * height);

        if (isLeaf) {
            for (int by = by0; by < by1; ++by) {
                for (int bx = bx0; bx < bx1; ++bx) {
                    if (bx >= 0 && bx < width && by >= 0 && by < height) {
                        buffer[by * width + bx] = temperature;
                    }
                }
            }
        } else {
            for (int i = 0; i < NUM_CHILDREN; ++i) {
                if (children[i]) {
                    children[i]->fillUniformGrid(buffer, width, height, maxLevel);
                }
            }
        }
    }

    int getMaxLevel() const {
        if (isLeaf) return level;
        int maxChildLevel = level;
        for (int i = 0; i < NUM_CHILDREN; ++i) {
            if (children[i]) {
                int childLevel = children[i]->getMaxLevel();
                if (childLevel > maxChildLevel) maxChildLevel = childLevel;
            }
        }
        return maxChildLevel;
    }

    // ========================================================================
    // Neighbor Management
    // ========================================================================

    void clearNeighbors() {
        for (int i = 0; i < 4; ++i) {
            neighbors[i].clear();
        }
    }

    void buildNeighborsUsingGrid() {
        // Collect all leaf nodes
        std::vector<QuadTreeNode*> leaves;
        getAllLeaves(leaves);

        if (leaves.empty()) return;

        // Find the maximum level
        int maxLevel = 0;
        for (const auto* leaf : leaves) {
            if (leaf->level > maxLevel) {
                maxLevel = leaf->level;
            }
        }

        int gridSize = 1 << maxLevel;  // 2^maxLevel

        // Create the flat grid
        std::vector<std::vector<QuadTreeNode*>> flatGrid(
            gridSize,
            std::vector<QuadTreeNode*>(gridSize, nullptr)
        );

        // Place each leaf into ALL the spots it occupies
        for (auto* leaf : leaves) {
            int scale = 1 << (maxLevel - leaf->level);
            int x0 = leaf->gridX * scale;
            int y0 = leaf->gridY * scale;

            // Fill ALL positions this cell occupies
            for (int dy = 0; dy < scale; ++dy) {
                for (int dx = 0; dx < scale; ++dx) {
                    int x = x0 + dx;
                    int y = y0 + dy;
                    if (x >= 0 && x < gridSize && y >= 0 && y < gridSize) {
                        flatGrid[y][x] = leaf;
                    }
                }
            }
        }

        // Clear old neighbors
        for (auto* leaf : leaves) {
            leaf->clearNeighbors();
        }

        // Find neighbors for each leaf
        for (auto* leaf : leaves) {
            int scale = 1 << (maxLevel - leaf->level);
            int x0 = leaf->gridX * scale;
            int y0 = leaf->gridY * scale;

            // LEFT
            if (x0 > 0) {
                const int x = x0 - 1;
                for (int dy = 0; dy < scale; ++dy) {
                    const int y = y0 + dy;
                    if (y >= 0 && y < gridSize) {
                        QuadTreeNode* candidate = flatGrid[y][x];
                        if (candidate && candidate != leaf) {
                            leaf->addNeighbor(LEFT, candidate);
                        }
                    }
                }
            }

            // RIGHT
            if (x0 + scale < gridSize) {
                const int x = x0 + scale;
                for (int dy = 0; dy < scale; ++dy) {
                    const int y = y0 + dy;
                    if (y >= 0 && y < gridSize) {
                        QuadTreeNode* candidate = flatGrid[y][x];
                        if (candidate && candidate != leaf) {
                            leaf->addNeighbor(RIGHT, candidate);
                        }
                    }
                }
            }

            // TOP
            if (y0 > 0) {
                const int y = y0 - 1;
                for (int dx = 0; dx < scale; ++dx) {
                    const int x = x0 + dx;
                    if (x >= 0 && x < gridSize) {
                        QuadTreeNode* candidate = flatGrid[y][x];
                        if (candidate && candidate != leaf) {
                            leaf->addNeighbor(TOP, candidate);
                        }
                    }
                }
            }

            // BOTTOM
            if (y0 + scale < gridSize) {
                const int y = y0 + scale;
                for (int dx = 0; dx < scale; ++dx) {
                    const int x = x0 + dx;
                    if (x >= 0 && x < gridSize) {
                        QuadTreeNode* candidate = flatGrid[y][x];
                        if (candidate && candidate != leaf) {
                            leaf->addNeighbor(BOTTOM, candidate);
                        }
                    }
                }
            }
        }
    }

    // ========================================================================
    // Gradient Computation
    // ========================================================================

    /**
     * Compute the temperature gradient in the X direction (∂T/∂x)
     * Uses the LEFT and RIGHT neighbors
     * Returns 0.0 if no neighbors available
     */
    double computeGradientX() const {
        if (!isLeaf) return 0.0;

        // If we have multiple neighbors, average their positions
        // For now, use the first LEFT and RIGHT neighbors
        // (In AMR, we need to weight by distance)

        if (neighbors[LEFT].empty() || neighbors[RIGHT].empty()) {
            // Neumann boundary condition with zero flux (boundaries are insulated)
            return 0.0;
        }

        // Get the first neighbor in each direction
        // (This is a simplification - we'll handle multiple neighbors later)
        const QuadTreeNode* left = neighbors[LEFT][0];
        const QuadTreeNode* right = neighbors[RIGHT][0];

        // Approximate the distance between cells
        // For same-level neighbors, distance = 2 * cellSize
        // For mixed-level, this is more complex
        double cellSize = 1.0 / (1 << level);
        double dx = 2.0 * cellSize;

        return (right->temperature - left->temperature) / dx;
    }

    /**
     * Compute the temperature gradient in the Y direction (∂T/∂y)
     */
    double computeGradientY() const {
        if (!isLeaf) return 0.0;

        if (neighbors[TOP].empty() || neighbors[BOTTOM].empty()) {
            return 0.0;  // Boundary
        }

        const QuadTreeNode* top = neighbors[TOP][0];
        const QuadTreeNode* bottom = neighbors[BOTTOM][0];

        // Approximate
        double cellSize = 1.0 / (1 << level);
        double dy = 2.0 * cellSize;
        return (top->temperature - bottom->temperature) / dy;
    }

    /**
     * Compute the gradient magnitude: |∇T| = sqrt((∂T/∂x)² + (∂T/∂y)²)
     * This is the key metric for deciding where to refine
     */
    double computeGradientMagnitude() const {
        double dx = computeGradientX();
        double dy = computeGradientY();
        return std::sqrt(dx * dx + dy * dy);
    }

    /**
     * Compute the Laplacian: ∇²T = ∂²T/∂x² + ∂²T/∂y²
     * This is used in the heat equation: ∂T/∂t = α∇²T
     */
    double computeLaplacian() const {
        if (!isLeaf) return 0.0;

        // Accumulate temperature differences from all neighbors
        double sum = 0.0;
        int count = 0;

        // LEFT neighbors
        for (const auto* neighbor : neighbors[LEFT]) {
            sum += neighbor->temperature - temperature;
            count++;
        }

        // RIGHT neighbors
        for (const auto* neighbor : neighbors[RIGHT]) {
            sum += neighbor->temperature - temperature;
            count++;
        }

        // TOP neighbors
        for (const auto* neighbor : neighbors[TOP]) {
            sum += neighbor->temperature - temperature;
            count++;
        }

        // BOTTOM neighbors
        for (const auto* neighbor : neighbors[BOTTOM]) {
            sum += neighbor->temperature - temperature;
            count++;
        }

        // Average the contributions
        // For a uniform 2D grid with 4 neighbors, laplacian = sum(T_neighbor - T_center)
        // ∇²T ≈ (T_left + T_right + T_top + T_bottom - 4*T) / h²
        // With multiple neighbors, we average
        if (count == 0) return 0.0;

        // Normalize by count and divide by h²
        double cellSize = 1.0 / (1 << level);
        double h2 = cellSize * cellSize;

        return (4.0 * sum / count) / h2;
    }

private:
    // Child indices
    enum ChildIndex { NE = 0, SE = 1, SW = 2, NW = 3};

    void addNeighbor(Direction dir, QuadTreeNode* neighbor) {
        if (!neighbor || neighbor == this) return;

        // Check if already in the list
        for (const auto* existing : neighbors[dir]) {
            if (existing == neighbor) return;
        }
        neighbors[dir].push_back(neighbor);
    }
};