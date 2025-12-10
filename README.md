# Ukkonen's algorithm
A linear-time, online algorithm for constructing suffix trees


### Run for test


```bash
g++ -std=c++17 -O3 test_examples.cpp suffixtree.cpp -o ukkonen_examples
./ukkonen_examples
```


```bash
g++ -std=c++17 -O3 test_runtime.cpp suffixtree.cpp -o ukkonen_benchmark
./ukkonen_benchmark
```


```bash
g++ -std=c++17 -O3 -march=armv8-a+simd test_runtime_neon.cpp suffixtree_neon.cpp -o ukkonen_neon
./ukkonen_neon
```

```bash
g++ -std=c++17 -O3 -mavx2 -mbmi test_runtime_avx.cpp suffixtree_avx.cpp -o ukkonen_avx
./ukkonen_avx
```