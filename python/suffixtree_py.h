/**
 * @file suffixtree.h
 * @brief High-performance, header-only C++ implementation of Ukkonen's Suffix Tree Algorithm.
 * * Features:
 * - O(N) linear time construction logic.
 * - Header-only: Just include and use.
 * - Memory efficient node management.
 * - Includes search and visualization capabilities.
 */

#ifndef SUFFIX_TREE_H
#define SUFFIX_TREE_H

#include <iostream>
#include <string>
#include <map>
#include <vector>

class SuffixTree {
public:
    // --- Internal Data Structures ---
    
    struct Node {
        int start;
        int* end; 
        Node* suffixLink;
        std::map<char, Node*> children;
        int id; // For debugging / visualization

        Node(int start, int* end, int id) 
            : start(start), end(end), suffixLink(nullptr), id(id) {}
    };

    // --- Public Interface ---

    /**
     * @brief Construct a new Suffix Tree object
     * * @param t The input text. It is recommended to append a unique char like '$'.
     * If the string does not end with '$', this constructor appends it automatically.
     */
    SuffixTree(std::string t);

    /**
     * @brief Destroy the Suffix Tree object and clean up memory.
     */
    ~SuffixTree();

    /**
     * @brief Search for a specific substring pattern in the text.
     * @param pattern The substring to search for.
     * @return true if found, false otherwise.
     */
    bool search(std::string pattern);

    /**
     * @brief Print the tree structure to stdout for debugging.
     */
    void printTree();

    /**
     * @brief Returns the raw text used to build the tree.
     */
    std::string getText() const { return text; }

private:
    std::string text;
    int size;
    
    Node *root;
    Node *activeNode;
    
    // Global pointers for leaf and root ends
    int leafEnd;
    int *rootEnd;
    
    // Ukkonen's state variables
    int activeEdge;     // Index in 'text' representing the current edge char
    int activeLength;   // How far down the active edge we are
    int remainder;      // How many suffixes remain to be added
    int nodeCount;      // Helper for ID generation

    // --- Helper Functions ---
    Node* newNode(int start, int *end);
    void freeSuffixTreeByPostOrder(Node *n);
    int edgeLength(Node *n);
    bool walkDown(Node *n);
    void extend(int pos);
    
    // Recursive helpers
    void printRecursive(Node *n, int depth);
    bool searchRecursive(Node *n, std::string &pattern, int idx);
};

// =========================================================
// Implementation Details
// =========================================================

inline SuffixTree::SuffixTree(std::string t) : text(t) {
    // Ideally, append '$' if not present for proper suffix counting.
    if (text.empty() || text.back() != '$') {
        text += "$";
    }
    size = text.length();

    // Initialize state
    nodeCount = 0;
    leafEnd = -1;
    rootEnd = new int(-1); // Root's end is static
    
    // Create Root
    root = newNode(-1, rootEnd);
    root->suffixLink = root; // Root's suffix link points to itself
    
    activeNode = root;
    activeEdge = -1;
    activeLength = 0;
    remainder = 0;

    // Build the tree character by character
    for (int i = 0; i < size; i++) {
        extend(i);
    }
}

inline SuffixTree::~SuffixTree() {
    freeSuffixTreeByPostOrder(root);
    delete rootEnd;
}

inline SuffixTree::Node* SuffixTree::newNode(int start, int *end) {
    Node *node = new Node(start, end, nodeCount++);
    node->suffixLink = root; // Default to root
    return node;
}

inline void SuffixTree::freeSuffixTreeByPostOrder(Node *n) {
    if (!n) return;
    for (auto const& [key, child] : n->children) {
        freeSuffixTreeByPostOrder(child);
    }
    if (n->end != &leafEnd && n->end != rootEnd) {
        delete n->end; // Delete allocated int for internal nodes
    }
    delete n;
}

inline int SuffixTree::edgeLength(Node *n) {
    if (n == root) return 0;
    return *(n->end) - (n->start) + 1;
}

inline bool SuffixTree::walkDown(Node *n) {
    int len = edgeLength(n);
    if (activeLength >= len) {
        activeEdge += len;
        activeLength -= len;
        activeNode = n;
        return true;
    }
    return false;
}

inline void SuffixTree::extend(int pos) {
    // Rule 1: Extension. We increment the global leafEnd.
    // All leaf nodes' edges (which point to &leafEnd) automatically extend by 1.
    leafEnd = pos;
    
    // We have one more suffix to add (the one ending at 'pos')
    remainder++;
    
    Node *lastNewNode = nullptr; // To handle suffix links creation

    while (remainder > 0) {
        // If activeLength is 0, look for the current character from activeNode
        if (activeLength == 0) {
            activeEdge = pos;
        }

        // Identify the next node/edge we are looking at
        char currentEdgeChar = text[activeEdge];
        
        // If there is no edge starting with this character from activeNode
        if (activeNode->children.find(currentEdgeChar) == activeNode->children.end()) {
            // Rule 2: Create a new leaf node
            activeNode->children[currentEdgeChar] = newNode(pos, &leafEnd);

            // If we created a new internal node in the previous step, link it here
            if (lastNewNode != nullptr) {
                lastNewNode->suffixLink = activeNode;
                lastNewNode = nullptr;
            }
        } 
        else {
            // There is an edge. Let's see if we need to walk down it.
            Node *next = activeNode->children[currentEdgeChar];
            
            if (walkDown(next)) {
                // We walked down, start loop again from new activeNode
                continue; 
            }

            // We are inside an edge. Check if the character matches.
            // Edge starts at next->start. We want the character at index: start + activeLength
            if (text[next->start + activeLength] == text[pos]) {
                // Rule 3: Character matches. Current suffix exists implicitly.
                // We increment activeLength and STOP processing this phase (showstopper).
                
                if (lastNewNode != nullptr && activeNode != root) {
                    lastNewNode->suffixLink = activeNode;
                    lastNewNode = nullptr;
                }
                
                activeLength++;
                break; // Stop the while loop, proceed to next character in text
            }

            // Rule 2 (Split): Character mismatch. 
            // We must split the edge and create a new internal node.
            
            // 1. Create the internal split node
            // The split point is at 'next->start + activeLength - 1'
            int *splitEnd = new int(next->start + activeLength - 1);
            Node *split = newNode(next->start, splitEnd);
            
            // Replace the old full edge with the split edge in activeNode
            activeNode->children[currentEdgeChar] = split;

            // 2. Adjust the old node (next) to be a child of the split node
            next->start += activeLength; // Push start forward
            split->children[text[next->start]] = next;

            // 3. Create a new leaf node for the current character being added
            split->children[text[pos]] = newNode(pos, &leafEnd);

            // 4. Maintenance of Suffix Links
            if (lastNewNode != nullptr) {
                lastNewNode->suffixLink = split;
            }
            lastNewNode = split;
        }

        // Decrement remainder because we successfully added a suffix
        remainder--;

        // Rule 1 & 3 logic for updating activeNode and activeLength
        if (activeNode == root && activeLength > 0) {
            activeLength--;
            activeEdge = pos - remainder + 1; // Shift to next suffix start
        } else if (activeNode != root) {
            // Follow suffix link
            activeNode = activeNode->suffixLink;
        }
    }
}

inline void SuffixTree::printTree() {
    std::cout << "\n--- Suffix Tree Structure ---\n";
    printRecursive(root, 0);
    std::cout << "-----------------------------\n";
}

inline void SuffixTree::printRecursive(Node *n, int depth) {
    if (!n) return;
    
    // Print edge leading to this node
    if (n->start != -1) { // Skip root text print
        for (int i = 0; i < depth; i++) std::cout << "  ";
        
        int currentEnd = *(n->end);
        std::cout << "Edge [" << n->start << "," << currentEnd << "]: ";
        for (int i = n->start; i <= currentEnd; i++) {
            std::cout << text[i];
        }
        std::cout << " (Node " << n->id << ")" << std::endl;
    } else {
        std::cout << "Root (Node " << n->id << ")" << std::endl;
    }

    if (n->children.empty()) {
        return;
    }

    for (auto const& [key, child] : n->children) {
        printRecursive(child, depth + 1);
    }
}

inline bool SuffixTree::search(std::string pattern) {
    if (pattern.empty()) return true;
    return searchRecursive(root, pattern, 0);
}

inline bool SuffixTree::searchRecursive(Node *n, std::string &pattern, int idx) {
    if (idx >= (int)pattern.length()) return true;

    char charCode = pattern[idx];
    if (n->children.find(charCode) == n->children.end()) {
        return false; 
    }

    Node *child = n->children[charCode];
    int edgeLen = edgeLength(child);
    
    int matchLen = 0;
    for (int i = 0; i < edgeLen && (idx + i) < (int)pattern.length(); i++) {
        if (text[child->start + i] != pattern[idx + i]) {
            return false; 
        }
        matchLen++;
    }

    if (matchLen == edgeLen) {
        return searchRecursive(child, pattern, idx + edgeLen);
    } else if (matchLen + idx == (int)pattern.length()) {
        return true;
    }
    
    return false;
}

#endif // SUFFIX_TREE_H