#ifndef SUFFIX_TREE_AVX_H
#define SUFFIX_TREE_AVX_H

#include <string>
#include <vector>
#include <iostream>
#include <immintrin.h> 
#include <cstdint>

struct Node {
    int start;
    int *end;
    Node *suffixLink;
    int id;

    // Separate vectors for keys (chars) and pointers for SIMD/Cache efficiency
    std::vector<uint8_t> keys; 
    std::vector<Node*> children;

    Node(int start, int *end, int id) 
        : start(start), end(end), suffixLink(nullptr), id(id) {
            // Pre-allocate small capacity to avoid immediate realloc
            keys.reserve(4); 
            children.reserve(4);
        }
    
    void addChild(char c, Node* n) {
        keys.push_back((uint8_t)c);
        children.push_back(n);
    }
};

class SuffixTreeAVX {
public:
    SuffixTreeAVX(std::string text);
    ~SuffixTreeAVX();

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
    
    // --- AVX2 Helper ---
    Node* findChild(Node* n, char c);
    
    bool searchRecursive(Node *n, std::string &pattern, int idx);
};

#endif // SUFFIX_TREE_AVX_H