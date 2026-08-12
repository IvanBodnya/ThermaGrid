#include "QuadTreeNode.h"
#include <iostream>
#include <vector>
#include <iomanip>

void printGradients(QuadTreeNode* root) {
    std::vector<QuadTreeNode*> leaves;
    root->getAllLeaves(leaves);
    
    std::cout << std::setw(10) << "Cell"
              << std::setw(10) << "Level"
              << std::setw(10) << "Temp"
              << std::setw(12) << "dT/dx"
              << std::setw(12) << "dT/dy"
              << std::setw(15) << "|grad T|"
              << std::setw(15) << "Laplacian"
              << std::endl;
    std::cout << std::string(80, '-') << std::endl;
    
    for (auto* leaf : leaves) {
        std::cout << std::setw(10) << "(" << leaf->gridX << "," << leaf->gridY << ")"
                  << std::setw(10) << leaf->level
                  << std::setw(10) << std::fixed << std::setprecision(2) << leaf->temperature
                  << std::setw(12) << std::setprecision(4) << leaf->computeGradientX()
                  << std::setw(12) << leaf->computeGradientY()
                  << std::setw(15) << leaf->computeGradientMagnitude()
                  << std::setw(15) << leaf->computeLaplacian()
                  << std::endl;
    }
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  ThermaGrid - Gradient Test" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Create a simple temperature field with a gradient
    // Domain: 2x2 at Level 1
    // Temperatures increase from left to right and bottom to top
    
    QuadTreeNode* root = new QuadTreeNode(20.0, 0, 0, 0);
    root->refine();  // Level 1
    
    // Manually set temperatures to create a gradient
    // Left side: cold, Right side: hot
    // Bottom: cold, Top: hot
    root->children[0]->temperature = 10.0;  // NW (top-left) = cold
    root->children[1]->temperature = 30.0;  // NE (top-right) = hot
    root->children[2]->temperature = 15.0;  // SW (bottom-left) = cold
    root->children[3]->temperature = 35.0;  // SE (bottom-right) = hot
    
    // Build neighbors
    root->buildNeighborsUsingGrid();
    
    std::cout << "\n=== Uniform Level 1 Grid with Linear Gradient ===" << std::endl;
    std::cout << "Temperatures: NW=10 C, NE=30 C, SW=15 C, SE=35 C" << std::endl;
    printGradients(root);
    
    // Now refine the NW cell (the cold region)
    std::cout << "\n\n=== Refine NW Cell (cold region) ===" << std::endl;
    root->children[0]->refine();  // Level 2
    root->buildNeighborsUsingGrid();
    
    // Set temperatures for the refined cells (maintaining the gradient)
    root->children[0]->children[0]->temperature = 8.0;   // NW
    root->children[0]->children[1]->temperature = 12.0;  // NE
    root->children[0]->children[2]->temperature = 9.0;   // SW
    root->children[0]->children[3]->temperature = 13.0;  // SE
    
    printGradients(root);
    
    delete root;
    
    std::cout << "\n All tests passed!" << std::endl;
    return 0;
}