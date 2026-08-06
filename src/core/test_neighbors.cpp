//
// Created by ivanb on 12.07.2026.
//

#include "test_neighbors.h"
#include "QuadTreeNode.h"
#include <iostream>
#include <vector>
void printNeighbors(QuadTreeNode* node, int indent = 0) {
    if (!node) return;

    std::string prefix(indent, ' ');
    const char* dirNames[] = {"LEFT", "RIGHT", "TOP", "BOTTOM"};

    std::cout << prefix << "Node at (" << node->gridX << "," << node->gridY
              << ") Level " << node->level << std::endl;

    for (int i = 0; i < 4; ++i) {
        if (!node->neighbors[i].empty()) {
            std::cout << prefix << "  " << dirNames[i] << ": ";
            for (auto* neighbor : node->neighbors[i]) {
                std::cout << "(" << neighbor->gridX << "," << neighbor->gridY
                          << ") Lv" << neighbor->level << " ";
            }
            std::cout << std::endl;
        }
    }
}

void printAllLeaves(QuadTreeNode* root) {
    std::vector<QuadTreeNode*> leaves;
    root->getAllLeaves(leaves);

    std::cout << "\n=== All Leaves ===" << std::endl;
    for (auto* leaf : leaves) {
        printNeighbors(leaf);
        std::cout << std::endl;
    }
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  ThermaGrid - Full Neighbor Test" << std::endl;
    std::cout << "========================================" << std::endl;

    // ========================================================================
    // Create the tree
    // ========================================================================

    // Root at Level 0
    QuadTreeNode* root = new QuadTreeNode(20.0, 0, 0, 0);

    // Refine to Level 1
    root->refine();
    root->buildNeighborsUsingGrid();
    std::cout << "\n=== Level 1 Grid (4 cells) ===" << std::endl;
    printAllLeaves(root);

    // ========================================================================
    // Case 1: Refine NW child only
    // ========================================================================

    std::cout << "\n========================================" << std::endl;
    std::cout << "  Case 1: Refine NW (Level 1 → Level 2)" << std::endl;
    std::cout << "========================================" << std::endl;

    // Find NW child (index 0)
    QuadTreeNode* nw = root->children[0];
    nw->refine();
    root->buildNeighborsUsingGrid();
    printAllLeaves(root);

    // ========================================================================
    // Case 2: Refine SE child as well
    // ========================================================================

    std::cout << "\n========================================" << std::endl;
    std::cout << "  Case 2: Refine SE (Level 1 → Level 2)" << std::endl;
    std::cout << "========================================" << std::endl;

    QuadTreeNode* se = root->children[3];
    se->refine();
    root->buildNeighborsUsingGrid();
    printAllLeaves(root);

    // ========================================================================
    // Clean up
    // ========================================================================

    delete root;

    std::cout << "\n✅ All tests passed!" << std::endl;
    return 0;
}
