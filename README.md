## Fast Floyd Warshall
Optimized implementation of Floyd Warshall algorithm using modern AVX2 extended instruction set.

SIMD vector instructions allow efficient parallelization on the CPU level.

The results become even better when the graph weights are limited to the range of 8 or 16 bits.

## Results
Assembly implementation completely outperforms C++ implementation even when `-O3` compiler flag is specified.

This was tested on both sparse and dense graphs of large sizes.
### Comparison including C++
![GitHub Logo](/docs/with_cpp.png)
![GitHub Logo](/docs/with_cpp_4096.png)

### Comparison excluding C++
![GitHub Logo](/docs/without_cpp.png)
![GitHub Logo](/docs/without_cpp_4096.png)

November, 2020.
