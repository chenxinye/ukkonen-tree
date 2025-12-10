#include "suffixtree.h"

SuffixTree::SuffixTree(std::string t) : text(t) {
    // Append a unique terminal character usually, but here we assume 
    // the user might handle it or we process raw text. 
    // Ideally, append '$' if not present for proper suffix counting.
    if (text.empty() || text.back() != '$') {
        text += "$";
    }
    size = text.length();

    // Initialize state
    nodeCount = 0;
    leafEnd = -1;
    rootEnd = new int(-1); // Root's end doesn't matter much, usually -1
    
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

SuffixTree::~SuffixTree() {
    freeSuffixTreeByPostOrder(root);
    delete rootEnd;
}

Node* SuffixTree::newNode(int start, int *end) {
    Node *node = new Node(start, end, nodeCount++);
    node->suffixLink = root; // Default to root
    return node;
}

void SuffixTree::freeSuffixTreeByPostOrder(Node *n) {
    if (!n) return;
    for (auto const& [key, child] : n->children) {
        freeSuffixTreeByPostOrder(child);
    }
    if (n->end != &leafEnd && n->end != rootEnd) {
        delete n->end; // Delete allocated int for internal nodes
    }
    delete n;
}

int SuffixTree::edgeLength(Node *n) {
    if (n == root) return 0;
    return *(n->end) - (n->start) + 1;
}

/**
 * walkDown (Skip/Count Trick):
 * If activeLength is larger than the edge length of the current child,
 * we hop down to that child node and adjust active parameters.
 */
bool SuffixTree::walkDown(Node *n) {
    int len = edgeLength(n);
    if (activeLength >= len) {
        activeEdge += len;
        activeLength -= len;
        activeNode = n;
        return true;
    }
    return false;
}

/**
 * extend:
 * The heart of Ukkonen's algorithm. Adds character at text[pos] to the tree.
 */
void SuffixTree::extend(int pos) {
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

// --- Visualization and Search Helpers ---

void SuffixTree::printTree() {
    std::cout << "\n--- Suffix Tree Structure ---\n";
    printRecursive(root, 0);
    std::cout << "-----------------------------\n";
}

void SuffixTree::printRecursive(Node *n, int depth) {
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
        // Leaf node info could go here
        return;
    }

    // Sort keys for consistent printing (map does this automatically)
    for (auto const& [key, child] : n->children) {
        printRecursive(child, depth + 1);
    }
}

bool SuffixTree::search(std::string pattern) {
    if (pattern.empty()) return true;
    return searchRecursive(root, pattern, 0);
}

bool SuffixTree::searchRecursive(Node *n, std::string &pattern, int idx) {
    // If we have matched the full pattern, return true
    if (idx >= pattern.length()) return true;

    // Determine which edge to take
    char charCode = pattern[idx];
    if (n->children.find(charCode) == n->children.end()) {
        return false; // No edge starts with this char
    }

    Node *child = n->children[charCode];
    int edgeLen = edgeLength(child);
    
    // Match the pattern along this edge
    int matchLen = 0;
    for (int i = 0; i < edgeLen && (idx + i) < pattern.length(); i++) {
        if (text[child->start + i] != pattern[idx + i]) {
            return false; // Mismatch on edge
        }
        matchLen++;
    }

    // If we traversed the whole edge, recurse to next node
    if (matchLen == edgeLen) {
        return searchRecursive(child, pattern, idx + edgeLen);
    } 
    // If we finished the pattern inside this edge
    else if (matchLen + idx == pattern.length()) {
        return true;
    }
    
    // Mismatch inside edge (pattern continues but edge doesn't match)
    return false;
}