# Ukkonen's algorithm



Ukkonen's algorithm is a linear-time, online algorithm for constructing suffix trees. This project offers a highly optimized, linear-time O(n) implementation of Ukkonen’s Suffix Tree algorithm, featuring a clean C++17 implementation, SIMD-accelerated versions for ARM NEON and x86 AVX2, and Python bindings via pybind11 for seamless high-performance integration in Python.

### Run for test
Use the following commands to compile and run the tests based on your hardware platform.

Runs on any standard C++ compiler.
```bash
g++ -std=c++17 -O3 test_examples.cpp suffixtree.cpp -o ukkonen_examples
./ukkonen_examples
```

```bash
g++ -std=c++17 -O3 test_runtime.cpp suffixtree.cpp -o ukkonen_benchmark
./ukkonen_benchmark
```

Target Hardware: Apple Silicon (MacBook M1/M2/M3), Raspberry Pi 4/5, AWS Graviton.
```bash
g++ -std=c++17 -O3 -march=armv8-a+simd test_runtime_neon.cpp suffixtree_neon.cpp -o ukkonen_neon
./ukkonen_neon
```

Target Hardware: Intel Core i5/i7 (4th Gen+), AMD Ryzen.
```bash
g++ -std=c++17 -O3 -mavx2 -mbmi test_runtime_avx.cpp suffixtree_avx.cpp -o ukkonen_avx
./ukkonen_avx
```




### Python bindings

This library includes a setup.py script to compile the C++ core into a Python extension module.

Ensure you have pybind11 installed (or let pip handle it via pyproject.toml):
```bash
python setup.py build_ext --inplace
```

Alternative way:
```bash
pip install pyukkonen
```

The usage example is below

```python
import pyukkonen

# Initialize the tree (automatically appends '$' terminator)
text = "bananarama"
tree = pyukkonen.SuffixTree(text)

# Search for substrings
patterns = ["nana", "apple", "rama"]
for p in patterns:
    found = tree.search(p)
    print(f"Pattern '{p}': {'Found' if found else 'Not Found'}")

# Visualize the tree structure
tree.print_tree()
```


### License
MIT License. See LICENSE for details.