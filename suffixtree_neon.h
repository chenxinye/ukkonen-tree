#ifndef SUFFIX_TREE_NEON_H
#define SUFFIX_TREE_NEON_H

#include <string>
#include <vector>
#include <iostream>
#include <arm_neon.h> // Key header for SIMD intrinsics

struct Node {
    int start;
    int *end;
    Node *suffixLink;
    int id;

    // --- SIMD OPTIMIZATION ---
    // Instead of std::map, we store keys (chars) and values (Node*) 
    // in contiguous vectors. This allows us to load 'keys' into 
    // vector registers efficiently.
    std::vector<uint8_t> keys; 
    std::vector<Node*> children;

    Node(int start, int *end, int id) 
        : start(start), end(end), suffixLink(nullptr), id(id) {
            // Reserve some space to avoid reallocations
            keys.reserve(4); 
            children.reserve(4);
        }
    
    // Add a child (helper function)
    void addChild(char c, Node* n) {
        keys.push_back((uint8_t)c);
        children.push_back(n);
    }
};

class SuffixTreeNeon {
public:
    SuffixTreeNeon(std::string text);
    ~SuffixTreeNeon();

    bool search(std::string pattern);
    int getNodeCount() const { return nodeCount; }

private:
    std::string text;
    Node *root;
    
    Node *activeNode;
    int activeEdge;
    int activeLength;
    int remainder;
    
    int leafEnd;
    int *rootEnd;
    int size;
    int nodeCount;

    Node* newNode(int start, int *end);
    void freeSuffixTreeByPostOrder(Node *n);
    int edgeLength(Node *n);
    bool walkDown(Node *n);
    void extend(int pos);
    
    // --- SIMD Helper ---
    // Fast lookup using ARM NEON intrinsics
    Node* findChild(Node* n, char c);
    
    bool searchRecursive(Node *n, std::string &pattern, int idx);
};

#endif // SUFFIX_TREE_NEON_H