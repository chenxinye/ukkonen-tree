#include <iostream>
#include <vector>
#include <cassert>
#include "suffixtree.h"

// Helper function to run a test case
void runTest(std::string inputName, std::string text, const std::vector<std::string>& patterns, const std::vector<bool>& expectedResults) {
    std::cout << "Running Test: " << inputName << " (Text: \"" << text << "\")" << std::endl;
    
    // Construct Tree
    SuffixTree tree(text);
    
    // Optional: Print tree structure
    // tree.printTree();

    // Verification
    bool allPassed = true;
    for (size_t i = 0; i < patterns.size(); ++i) {
        bool found = tree.search(patterns[i]);
        if (found != expectedResults[i]) {
            std::cout << "  [FAIL] Pattern '" << patterns[i] << "'. Expected: " 
                      << (expectedResults[i] ? "True" : "False") 
                      << ", Got: " << (found ? "True" : "False") << std::endl;
            allPassed = false;
        } else {
            std::cout << "  [PASS] Pattern '" << patterns[i] << "'" << std::endl;
        }
    }
    
    if (allPassed) std::cout << ">> " << inputName << " Passed Complete.\n" << std::endl;
    else std::cout << ">> " << inputName << " FAILED.\n" << std::endl;
}

int main() {
    std::cout << "============================================" << std::endl;
    std::cout << "    Ukkonen's Suffix Tree Implementation    " << std::endl;
    std::cout << "============================================" << std::endl;

    // TEST CASE 1: Simple Alphabet
    // Note: The implementation appends '$' automatically.
    std::vector<std::string> patterns1 = {"abc", "bc", "c", "ab", "a", "d", "abd"};
    std::vector<bool> results1 = {true, true, true, true, true, false, false};
    runTest("Simple ABC", "abc", patterns1, results1);

    // TEST CASE 2: Repeats (The 'banana' problem)
    // Suffixes: banana$, anana$, nana$, ana$, na$, a$, $
    std::vector<std::string> patterns2 = {"ana", "nan", "banana", "ban", "xyz", "nana"};
    std::vector<bool> results2 = {true, true, true, true, false, true};
    runTest("Banana Test", "banana", patterns2, results2);

    // TEST CASE 3: Complex Repeats
    // Text: mississippi
    std::vector<std::string> patterns3 = {"issi", "ssi", "sip", "ippi", "miss", "m", "pp", "sis"};
    std::vector<bool> results3 = {true, true, true, true, true, true, true, true};
    runTest("Mississippi Test", "mississippi", patterns3, results3);

    // TEST CASE 4: Edge Case - Empty String (or just $)
    // Code handles empty by making it "$"
    SuffixTree emptyTree("");
    if(emptyTree.search("$")) std::cout << ">> Empty String Test Passed.\n" << std::endl;
    else std::cout << ">> Empty String Test Failed.\n" << std::endl;

    // TEST CASE 5: Visual Verification
    std::cout << ">> Visual Verification for 'xabxa':" << std::endl;
    SuffixTree visTree("xabxa");
    visTree.printTree();

    return 0;
}