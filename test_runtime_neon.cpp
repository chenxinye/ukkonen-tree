#include <iostream>
#include <chrono>
#include <random>
#include "suffixtree_neon.h"

// Generate random ASCII (32-126) to force wider branching factors
// SIMD benefits most when nodes have MANY children.
std::string generateRandomText(int length) {
    std::string result;
    result.resize(length);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(33, 126); // Readable ASCII

    for (int i = 0; i < length; ++i) {
        result[i] = (char)dis(gen);
    }
    return result;
}

int main() {
    int len = 500000; // 500k characters
    std::cout << "\n--- SIMD Test (Length: " << len << ") ---\n" << std::endl;
    std::cout << "Generating " << len << " random characters..." << std::endl;
    std::string text = generateRandomText(len);

    std::cout << "Building Suffix Tree with NEON Optimizations..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    
    SuffixTreeNeon tree(text);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    std::cout << "Construction Time: " << elapsed.count() << " ms" << std::endl;
    std::cout << "Nodes: " << tree.getNodeCount() << std::endl;
    
    return 0;
}