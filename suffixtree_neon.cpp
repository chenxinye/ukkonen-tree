#include "suffixtree_neon.h"

SuffixTreeNeon::SuffixTreeNeon(std::string t) : text(t) {
    if (text.empty() || text.back() != '$') {
        text += "$";
    }
    size = text.length();
    nodeCount = 0;
    leafEnd = -1;
    rootEnd = new int(-1);
    
    root = newNode(-1, rootEnd);
    root->suffixLink = root;
    activeNode = root;
    activeEdge = -1;
    activeLength = 0;
    remainder = 0;

    for (int i = 0; i < size; i++) {
        extend(i);
    }
}

SuffixTreeNeon::~SuffixTreeNeon() {
    freeSuffixTreeByPostOrder(root);
    delete rootEnd;
}

Node* SuffixTreeNeon::newNode(int start, int *end) {
    Node *node = new Node(start, end, nodeCount++);
    node->suffixLink = root; 
    return node;
}

void SuffixTreeNeon::freeSuffixTreeByPostOrder(Node *n) {
    if (!n) return;
    for (Node* child : n->children) {
        freeSuffixTreeByPostOrder(child);
    }
    if (n->end != &leafEnd && n->end != rootEnd) {
        delete n->end; 
    }
    delete n;
}

int SuffixTreeNeon::edgeLength(Node *n) {
    if (n == root) return 0;
    return *(n->end) - (n->start) + 1;
}

// --- NEON SIMD IMPLEMENTATION STARTS HERE ---

/**
 * findChild:
 * Uses ARM NEON intrinsics to search for character 'c' in the n->keys vector.
 * It processes 16 characters at a time.
 */
Node* SuffixTreeNeon::findChild(Node* n, char c) {
    size_t count = n->keys.size();
    if (count == 0) return nullptr;

    const uint8_t* ptr = n->keys.data();
    uint8_t target = (uint8_t)c;
    
    size_t i = 0;

    // 1. Vectorized Loop: Process 16 bytes at a time
    // Only worth it if we have >= 16 children. 
    // (Optimization hint: usually heavy branching nodes benefit most)
    if (count >= 16) {
        // Broadcast the target character to all 16 lanes of a 128-bit register
        uint8x16_t targetVec = vdupq_n_u8(target);

        for (; i <= count - 16; i += 16) {
            // Load 16 keys from memory unaligned
            uint8x16_t dataVec = vld1q_u8(ptr + i);
            
            // Compare: result is 0xFF where equal, 0x00 otherwise
            uint8x16_t cmp = vceqq_u8(dataVec, targetVec);
            
            // Reduction: Check if any byte in 'cmp' is non-zero.
            // 'vmaxvq_u8' finds the max value across the vector.
            // If max is 0, then no match found in this block.
            if (vmaxvq_u8(cmp) != 0) {
                // Match found in this block! Now we need the exact index.
                // Since we don't expect many hash collisions in a suffix tree node,
                // we can just fall back to scalar for this 16-byte block 
                // or use bit manipulation (more complex on ARM than x86).
                // Let's do a quick scalar scan on these 16 bytes.
                for (int j = 0; j < 16; ++j) {
                    if (ptr[i + j] == target) {
                        return n->children[i + j];
                    }
                }
            }
        }
    }

    // 2. Scalar Loop: Handle remaining elements (or if count < 16)
    for (; i < count; ++i) {
        if (ptr[i] == target) {
            return n->children[i];
        }
    }

    return nullptr;
}
// --- NEON SIMD IMPLEMENTATION ENDS HERE ---

bool SuffixTreeNeon::walkDown(Node *n) {
    int len = edgeLength(n);
    if (activeLength >= len) {
        activeEdge += len;
        activeLength -= len;
        activeNode = n;
        return true;
    }
    return false;
}

void SuffixTreeNeon::extend(int pos) {
    leafEnd = pos;
    remainder++;
    Node *lastNewNode = nullptr;

    while (remainder > 0) {
        if (activeLength == 0) activeEdge = pos;

        char currentEdgeChar = text[activeEdge];
        
        // REPLACED: map.find -> findChild (SIMD)
        Node* next = findChild(activeNode, currentEdgeChar);

        if (next == nullptr) {
            // Create new leaf
            // REPLACED: map insert -> addChild
            activeNode->addChild(currentEdgeChar, newNode(pos, &leafEnd));

            if (lastNewNode != nullptr) {
                lastNewNode->suffixLink = activeNode;
                lastNewNode = nullptr;
            }
        } 
        else {
            if (walkDown(next)) continue; 

            if (text[next->start + activeLength] == text[pos]) {
                if (lastNewNode != nullptr && activeNode != root) {
                    lastNewNode->suffixLink = activeNode;
                    lastNewNode = nullptr;
                }
                activeLength++;
                break; 
            }

            // Split
            int *splitEnd = new int(next->start + activeLength - 1);
            Node *split = newNode(next->start, splitEnd);
            
            // Need to update the child in the vector. 
            // We know 'next' corresponds to 'currentEdgeChar'.
            // Efficient linear scan to replace pointer (since we are here, it exists)
            for (size_t k = 0; k < activeNode->keys.size(); ++k) {
                if (activeNode->keys[k] == (uint8_t)currentEdgeChar) {
                    activeNode->children[k] = split;
                    break;
                }
            }

            next->start += activeLength; 
            split->addChild(text[next->start], next);
            split->addChild(text[pos], newNode(pos, &leafEnd));

            if (lastNewNode != nullptr) {
                lastNewNode->suffixLink = split;
            }
            lastNewNode = split;
        }

        remainder--;
        if (activeNode == root && activeLength > 0) {
            activeLength--;
            activeEdge = pos - remainder + 1; 
        } else if (activeNode != root) {
            activeNode = activeNode->suffixLink;
        }
    }
}

bool SuffixTreeNeon::search(std::string pattern) {
    if (pattern.empty()) return true;
    return searchRecursive(root, pattern, 0);
}

bool SuffixTreeNeon::searchRecursive(Node *n, std::string &pattern, int idx) {
    if (idx >= pattern.length()) return true;

    char charCode = pattern[idx];
    // REPLACED: map.find -> findChild (SIMD)
    Node *child = findChild(n, charCode);
    
    if (!child) return false;

    int edgeLen = edgeLength(child);
    int matchLen = 0;
    for (int i = 0; i < edgeLen && (idx + i) < pattern.length(); i++) {
        if (text[child->start + i] != pattern[idx + i]) return false;
        matchLen++;
    }

    if (matchLen == edgeLen) return searchRecursive(child, pattern, idx + edgeLen);
    else if (matchLen + idx == pattern.length()) return true;
    
    return false;
}