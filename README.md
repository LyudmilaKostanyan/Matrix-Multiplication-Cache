# Matrix Multiplication Performance Comparison

## Problem Description
This project implements and compares four different approaches to matrix multiplication:

1. **Naive Matrix Multiplication**  
   A straightforward implementation using three nested loops.

2. **Blocked Matrix Multiplication**  
   A cache-aware approach that divides matrices into fixed-size blocks to improve memory locality.

3. **Recursive Matrix Multiplication**  
   A divide-and-conquer algorithm that recursively splits matrices into quadrants until a base case is reached.

4. **Parallel Blocked Matrix Multiplication**  
   An extension of the blocked algorithm that distributes block-wise computation across multiple threads to utilize all CPU cores and increase throughput. It combines cache-aware access patterns with parallel processing for faster performance on multi-core machines.

The goal is to evaluate how these methods perform in terms of execution time, focusing on their interaction with the CPU cache (L1 cache size of 48 KB in this case). The program measures and outputs the time taken by each method to multiply two square matrices of size `N × N`, allowing customization of matrix size (`N`), block size (`BLOCK_SIZE`), and recursive base case (`BASE_CASE`) via command-line arguments.

---

## Example Output
Below is a sample output of the program with default parameters:

```
Parameters:
-----------------------------------
Matrix size  | 1024 x 1024
Block size   | 32
Base case    | 45
-----------------------------------
Method                   Time (ms)
----------------------------------------
Naive                    4118
Blocked                  1917
Parallel Blocked          814
Recursive                1947
```

- **Matrix size**: The dimensions of the matrices being multiplied (default: 1024 × 1024).
- **Block size**: The size of blocks used in the blocked multiplication (default: 32).
- **Base case**: The size at which the recursive algorithm switches to naive multiplication (default: 45).
- **Time (ms)**: Execution time in milliseconds for each method.

---

## Why Do Multiplication Methods Differ?

The performance differences between these methods stem from how they utilize the CPU cache and handle memory access patterns. Two key concepts in this context are **Cache-Aware** and **Cache-Oblivious** algorithms, which apply to the blocked and recursive methods, respectively.

### 1. Naive Matrix Multiplication
- **How it works**: Uses three nested loops to compute each element of the result matrix `C` by iterating over rows of `A` and columns of `B`.
- **Performance**: Slowest due to poor cache utilization. Accesses to `B` are column-wise (stride-N), causing frequent cache misses as data is not contiguous in memory.
- **Cache impact**: High number of cache misses, especially for large matrices. This method is neither cache-aware nor cache-oblivious—it ignores cache entirely.

### 2. Blocked Matrix Multiplication (Cache-Aware)
- **How it works**: Divides matrices into fixed-size blocks (e.g., 32 × 32) and multiplies these blocks, keeping data accesses within small, cache-friendly regions.
- **Cache-Aware Explanation**: This is a **cache-aware** algorithm because it requires knowledge of the cache size to determine an optimal `BLOCK_SIZE`. For example, a 32 × 32 block (8192 bytes) fits well within the 48 KB L1 cache when considering partial loading (three blocks = 24 KB).
- **Performance**: Faster than naive because it improves spatial and temporal locality, reducing cache misses significantly. However, its efficiency depends on choosing the right `BLOCK_SIZE`.
- **Cache impact**: Fewer misses due to localized access, but performance is sensitive to hardware-specific tuning.

### 3. Recursive Matrix Multiplication (Cache-Oblivious)
- **How it works**: Recursively splits matrices into four quadrants until reaching a base case (e.g., 45 × 45), then uses naive multiplication.
- **Cache-Oblivious Explanation**: This is a **cache-oblivious** algorithm because it doesn’t require prior knowledge of cache parameters. The recursive division naturally adapts to any cache hierarchy.
- **Performance**: Performs better than naive, but in this test case is **slightly slower than blocked** (1947 ms vs 1917 ms). This is likely due to overhead from many recursive calls and imperfect division for some matrix sizes.
- **Cache impact**: Good cache behavior for smaller submatrices, but performance can be limited by recursion overhead.

### 4. Parallel Blocked Matrix Multiplication
- **How it works**: Based on the blocked algorithm, but parallelized using multiple threads. Each thread handles a different block row section.
- **Cache & Thread Efficiency**: Threads work on distinct memory regions, minimizing false sharing and allowing per-core cache utilization. Blocked access patterns remain intact.
- **Performance**: **Fastest** in the example (814 ms), showing excellent scalability across CPU cores. Speedup depends on core count and memory bandwidth.
- **Trade-offs**: Increased memory pressure and synchronization complexity, but greatly improves overall throughput.

---

### Cache-Aware vs. Cache-Oblivious

| Type           | Description                                             | Tuned?           | Portability | Overhead    |
|----------------|---------------------------------------------------------|------------------|-------------|-------------|
| Cache-Aware    | Manually optimized to fit known cache (e.g., BLOCK_SIZE) | Requires tuning  | Low         | Low         |
| Cache-Oblivious| Recursive strategy that fits cache levels implicitly     | No tuning needed | High        | Medium-High |
| Parallel Blocked| Combines blocked access with multithreading             | Scales with CPU  | Medium      | Medium      |

> In the example output, **Parallel Blocked (814 ms)** is the fastest method, significantly outperforming **Blocked (1917 ms)**, **Recursive (1947 ms)**, and **Naive (4118 ms)**. This demonstrates the effectiveness of combining cache-aware blocking with thread-level parallelism.

---

## How to Build and Run

### 1. Clone the Repository
```bash
git clone https://github.com/LyudmilaKostanyan/Matrix-Multiplication-Cache.git
cd Matrix-Multiplication-Cache
```

### 2. Build the Project
Use CMake to build the project:
```bash
cmake -S . -B build
cmake --build build
```
Ensure you have CMake and a C++ compiler (e.g., g++) installed.

### 3. Run the Program

#### For Windows Users
Example with arguments:
```bash
./build/main.exe --block-size 64 --base-case 32 --n 512
```
Example without arguments:
```bash
./build/main.exe
```

#### For Linux/macOS Users
Example with arguments:
```bash
./build/main --block-size 64 --base-case 32 --n 512
```
Or without arguments:
```bash
./build/main
```

---

### Explanation of Arguments

| Argument        | Description                                                       |
|----------------|-------------------------------------------------------------------|
| `--block-size` | Sets the block size used in blocked and parallel blocked methods. |
| `--base-case`  | Threshold size for recursive base-case computation.               |
| `--n`          | Size of the square matrices `N × N`.                              |

---

### Default Values

- `N = 1024`
- `BLOCK_SIZE = 32`
- `BASE_CASE = 45`

---

### Why `BASE_CASE = 45`?
- L1 cache size = 48 KB
- 3 matrices of size 45 × 45 × 8 bytes = 47.46 KB
- Fits in L1 cache to reduce misses and recursion overhead.

### Why `BLOCK_SIZE = 32`?
- 32 × 32 × 8 = 8 KB per block
- 3 blocks = 24 KB, safely fits in 48 KB
- Offers good locality and low loop overhead.
