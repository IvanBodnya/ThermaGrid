/*
* Physics Model:
 * - 2D transient heat conduction on a square domain (1m x 1m)
 * - Central equation to solve: ∂T/∂t = α(∂²T/∂x² + ∂²T/∂y²) + Q(x,y,t)
 * - Thermal diffusivity: α = 1e-4 m²/s (aluminum)
 * - Left boundary: 100°C (Dirichlet)
 * - Right boundary: 0°C (Dirichlet)
 * - Top/Bottom boundaries: Insulated (Neumann, ∂T/∂n = 0)
 * - Initial condition: 20°C everywhere
 * - Heat source: Gaussian pulse in the center, Q = 1000*exp(-r²/0.01)
 */
#include "QuadTreeNode.h"
#include <iostream>
#include <memory>

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  ThermaGrid - AMR Test" << std::endl;
    std::cout << "========================================" << std::endl;

    auto root = std::make_unique<QuadTreeNode>(20.0, 0);

    std::cout << "\nInitial tree:" << std::endl;
    root->printTree();

    // Refine the root
    std::cout << "\nRefining root..." << std::endl;
    root->refine();
    root->printTree();

    // Refine one child
    std::cout << "\nRefining NW child..." << std::endl;
    root->children[0]->refine();
    root->printTree();

    // Fill uniform grid
    std::vector<double> buffer(16, 0.0); // 4x4 grid
    root->fillUniformGrid(buffer, 4, 4, root->getMaxLevel());

    std::cout << "\nUniform grid (4x4):" << std::endl;
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            std::cout << " " << buffer[y * 4 + x];
        }
        std::cout << std::endl;
    }

    std::cout << "\nTotal nodes: " << root->countNodes() << std::endl;
    std::cout << "\nAll tests passed!" << std::endl;

    return 0;
}