#include <iostream>
#include <chrono>
#include <random>
#include "suffixtree_avx.h"

// Generate random ASCII to encourage high branching factor
// where AVX2 32-byte scan shines.
std::string generateRandomText(int length) {
    std::string result;
    result.resize(length);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(33, 126); 

    for (int i = 0; i < length; ++i) {
        result[i] = (char)dis(gen);
    }
    return result;
}

int main() {
    // Large test size to see difference
    int len = 500000; // 500k characters
    std::cout << "Generating " << len << " random ASCII characters..." << std::endl;
    std::string text = generateRandomText(len);

    std::cout << "Building Suffix Tree with AVX2 Optimizations..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    
    SuffixTreeAVX tree(text);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    std::cout << "Construction Time: " << elapsed.count() << " ms" << std::endl;
    std::cout << "Nodes Created: " << tree.getNodeCount() << std::endl;
    
    // Quick verification
    std::string pattern = text.substr(len/2, 20);
    bool found = tree.search(pattern);
    std::cout << "Sanity Check (Search): " << (found ? "Passed" : "Failed") << std::endl;

    return 0;
}