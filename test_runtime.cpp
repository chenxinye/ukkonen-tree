#include <iostream>
#include <vector>
#include <string>
#include <chrono>  
#include <random>   
#include "suffixtree.h" 



void runCorrectnessTest() {
    std::cout << "\n--- Correctness Tests ---" << std::endl;
    std::string text = "banana";
    SuffixTree tree(text);
    
    std::vector<std::string> patterns = {"ana", "nan", "banana", "xyz"};
    for (const auto& pat : patterns) {
        std::cout << "Searching '" << pat << "': " 
                  << (tree.search(pat) ? "Found" : "Not Found") << std::endl;
    }
}


// Generates a random DNA string (A, C, G, T) of given length
std::string generateRandomDNA(int length) {
    const char charset[] = "ACGT";
    std::string result;
    result.resize(length);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 3);

    for (int i = 0; i < length; ++i) {
        result[i] = charset[dis(gen)];
    }
    return result;
}

// Function to measure construction time
void runPerformanceTest(int length) {
    std::cout << "\n--- Performance Test (Length: " << length << ") ---" << std::endl;
    
    std::string bigText = generateRandomDNA(length);
    
    // 1. Measure Construction Time
    auto start = std::chrono::high_resolution_clock::now();
    
    SuffixTree tree(bigText);
    
    auto end = std::chrono::high_resolution_clock::now();
    
    // Calculate duration in milliseconds
    std::chrono::duration<double, std::milli> elapsed = end - start;
    
    std::cout << "Text Generation: Done." << std::endl;
    std::cout << "Construction Time: " << elapsed.count() << " ms" << std::endl;
    std::cout << "Total Nodes Created: " << tree.getNodeCount() << std::endl;

    // 2. Measure Search Time (Verification)
    // Let's search for a pattern we know exists (end of the string)
    std::string pattern = bigText.substr(length - 10, 10); 
    
    auto searchStart = std::chrono::high_resolution_clock::now();
    bool found = tree.search(pattern);
    auto searchEnd = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double, std::nano> searchElapsed = searchEnd - searchStart;

    std::cout << "Search Time (10 chars): " << searchElapsed.count() << " ns" << std::endl;
    std::cout << "Pattern Found: " << (found ? "Yes" : "No") << std::endl;
}

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



void simd_comparison() {
    int len = 500000; // 500k characters
    std::cout << "\n--- SIMD Test (Length: " << len << ") ---\n" << std::endl;
    std::cout << "Generating " << len << " random characters..." << std::endl;
    std::string text = generateRandomText(len);

    std::cout << "Building Suffix Tree with NEON Optimizations..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    
    SuffixTree tree(text);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    std::cout << "Construction Time: " << elapsed.count() << " ms" << std::endl;
    std::cout << "Nodes: " << tree.getNodeCount() << std::endl;
    
}

int main() {

    runCorrectnessTest(); // 1. Basic Correctness Test

    // Ukkonen is O(N). Time should increase roughly linearly with size.

    // 2. Performance Benchmarks
    runPerformanceTest(10000);  // Small: 10,000 characters
     
    runPerformanceTest(100000); // Medium: 100,000 characters

    // Note: If running in debug mode, this might be slow. 
    runPerformanceTest(1000000); // Large: 1,000,000 characters


    simd_comparison();
    return 0;
}