#include "suffixtree_avx.h"

// Helper for bit manipulation (Count Trailing Zeros)
// _tzcnt_u32 is usually available in immintrin.h via BMI1
// If not, __builtin_ctz is the GCC/Clang intrinsic.
#ifdef _MSC_VER
#include <intrin.h>
inline int countTrailingZeros(uint32_t mask) {
    unsigned long index;
    _BitScanForward(&index, mask);
    return (int)index;
}
#else
inline int countTrailingZeros(uint32_t mask) {
    return __builtin_ctz(mask);
}
#endif

SuffixTreeAVX::SuffixTreeAVX(std::string t) : text(t) {
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

SuffixTreeAVX::~SuffixTreeAVX() {
    freeSuffixTreeByPostOrder(root);
    delete rootEnd;
}

Node* SuffixTreeAVX::newNode(int start, int *end) {
    Node *node = new Node(start, end, nodeCount++);
    node->suffixLink = root; 
    return node;
}

void SuffixTreeAVX::freeSuffixTreeByPostOrder(Node *n) {
    if (!n) return;
    for (Node* child : n->children) {
        freeSuffixTreeByPostOrder(child);
    }
    if (n->end != &leafEnd && n->end != rootEnd) {
        delete n->end; 
    }
    delete n;
}

int SuffixTreeAVX::edgeLength(Node *n) {
    if (n == root) return 0;
    return *(n->end) - (n->start) + 1;
}

// --- AVX2 IMPLEMENTATION STARTS HERE ---

/**
 * findChild (AVX2 Version):
 * Scans 32 characters at a time.
 */
Node* SuffixTreeAVX::findChild(Node* n, char c) {
    size_t count = n->keys.size();
    if (count == 0) return nullptr;

    const uint8_t* ptr = n->keys.data();
    uint8_t target = (uint8_t)c;
    size_t i = 0;

    // 1. AVX2 Loop: Process 32 bytes at a time
    // Only proceed if we have >= 32 elements to avoid reading OOB 
    // (though safe on stack/heap usually, strict bounds are safer).
    if (count >= 32) {
        // Broadcast target char to all 32 bytes of YMM register
        __m256i targetVec = _mm256_set1_epi8(target);

        for (; i <= count - 32; i += 32) {
            // Load 32 bytes (unaligned load is fine on modern AVX2 CPUs)
            __m256i dataVec = _mm256_loadu_si256((const __m256i*)(ptr + i));

            // Compare: Result is 0xFF where equal, 0x00 otherwise
            __m256i cmpVec = _mm256_cmpeq_epi8(dataVec, targetVec);

            // MoveMask: Extracts the most significant bit of each byte 
            // into a 32-bit integer.
            uint32_t mask = (uint32_t)_mm256_movemask_epi8(cmpVec);

            if (mask != 0) {
                // Found match(es)!
                // countTrailingZeros returns the index of the first set bit (0-31)
                // This corresponds EXACTLY to the byte index in the vector.
                int bitIndex = countTrailingZeros(mask);
                return n->children[i + bitIndex];
            }
        }
    }

    // 2. Scalar Fallback
    // Handles remaining elements or small nodes (< 32 children)
    for (; i < count; ++i) {
        if (ptr[i] == target) {
            return n->children[i];
        }
    }

    return nullptr;
}
// --- AVX2 IMPLEMENTATION ENDS HERE ---

bool SuffixTreeAVX::walkDown(Node *n) {
    int len = edgeLength(n);
    if (activeLength >= len) {
        activeEdge += len;
        activeLength -= len;
        activeNode = n;
        return true;
    }
    return false;
}

void SuffixTreeAVX::extend(int pos) {
    leafEnd = pos;
    remainder++;
    Node *lastNewNode = nullptr;

    while (remainder > 0) {
        if (activeLength == 0) activeEdge = pos;

        char currentEdgeChar = text[activeEdge];
        
        // Use AVX2 find
        Node* next = findChild(activeNode, currentEdgeChar);

        if (next == nullptr) {
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

            int *splitEnd = new int(next->start + activeLength - 1);
            Node *split = newNode(next->start, splitEnd);
            
            // Linear scan for replacement is still needed, but fast for small arrays
            // For huge branching factors, you could use AVX2 here too, 
            // but finding 'next' is usually sufficient.
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

bool SuffixTreeAVX::search(std::string pattern) {
    if (pattern.empty()) return true;
    return searchRecursive(root, pattern, 0);
}

bool SuffixTreeAVX::searchRecursive(Node *n, std::string &pattern, int idx) {
    if (idx >= pattern.length()) return true;

    char charCode = pattern[idx];
    Node *child = findChild(n, charCode); // Use AVX2 find
    
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