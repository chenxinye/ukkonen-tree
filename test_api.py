import pyukkonen

# 1. Create the suffix tree
# Note: No need to manually add '$', the C++ constructor handles it automatically
text = "bananarama"
print(f"正在构建后缀树: {text}")
tree = pyukkonen.SuffixTree(text)

# 2. Search substrings (returns True/False)
patterns = ["nana", "rama", "apple", "ban"]

print("\n--- 搜索测试 ---")
for p in patterns:
    found = tree.search(p)
    result = "找到了" if found else "没找到"
    print(f"搜索 '{p}': {result}")

# 3. Print the tree structure (output to console)
# This invokes C++ std::cout and displays the tree structure directly in the terminal
print("\n--- 树结构可视化 ---")
tree.print_tree()

# 4. Get the original text
print(f"\n原始文本: {tree.get_text()}")
