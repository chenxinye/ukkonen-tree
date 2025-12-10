#ifndef SUFFIX_TREE_H
#define SUFFIX_TREE_H

#include <string>
#include <map>
#include <vector>
#include <iostream>

/**
 * Node structure for the Suffix Tree.
 */
struct Node {
    // [start, *end] represents the substring on the edge leading to this node.
    // We use a pointer for 'end' to achieve O(1) extension for leaf nodes (Rule 1).
    int start;
    int *end;

    // Suffix Link used for fast traversal (Ukkonen's optimization)
    Node *suffixLink;

    // Unique ID for visualization/debugging
    int id;

    // Map of characters to child nodes (edges)
    std::map<char, Node*> children;

    // Constructor
    Node(int start, int *end, int id) 
        : start(start), end(end), suffixLink(nullptr), id(id) {}
};

/**
 * SuffixTree Class implementing Ukkonen's Algorithm.
 */
class SuffixTree {
public:
    // Constructor: Builds the tree immediately from the text
    SuffixTree(std::string text);
    
    // Destructor: Cleans up memory
    ~SuffixTree();

    // Utility: Visualization (Printing the tree structure)
    void printTree();

    // Utility: Search if a pattern exists in the text
    bool search(std::string pattern);
    int getNodeCount() const { return nodeCount; }

private:
    std::string text;
    Node *root;
    
    // -- Ukkonen's Algorithm State Variables --
    
    Node *activeNode;    // The node from which we are currently traversing
    int activeEdge;      // The index of the character in 'text' indicating the edge we are on
    int activeLength;    // How far down the activeEdge we are
    int remainder;       // How many suffixes remain to be inserted
    
    int leafEnd;         // Global end index for leaf nodes (updates every phase)
    int *rootEnd;        // Special end pointer for the root
    int *splitEnd;       // Helper pointer for internal split nodes
    int size;            // Length of input text
    int nodeCount;       // Counter to assign IDs to nodes

    // -- Internal Helper Functions --
    
    Node* newNode(int start, int *end);
    void freeSuffixTreeByPostOrder(Node *n);
    
    // Calculates the length of the edge leading to node n
    int edgeLength(Node *n);
    
    // Skips through nodes if activeLength is greater than current edge length
    bool walkDown(Node *n);
    
    // The core extension function called for every character
    void extend(int pos);
    
    // Helper for printing
    void printRecursive(Node *n, int depth);
    
    // Helper for searching
    bool searchRecursive(Node *n, std::string &pattern, int idx);
};

#endif // SUFFIX_TREE_H