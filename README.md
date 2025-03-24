# Matrix Multiplication Performance Comparison

## Problem Description
This project implements and compares three different approaches to matrix multiplication:
1. **Naive Matrix Multiplication**: A straightforward implementation using three nested loops.
2. **Blocked Matrix Multiplication**: A cache-aware approach that divides matrices into fixed-size blocks to improve memory locality.
3. **Recursive Matrix Multiplication**: A divide-and-conquer algorithm that recursively splits matrices into quadrants until a base case is reached.

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
Method              Time (ms)
-----------------------------------
Naive               1294
Blocked             914
Recursive           774
```

- **Matrix size**: The dimensions of the matrices being multiplied (default: 1024 × 1024).
- **Block size**: The size of blocks used in the blocked multiplication (default: 32).
- **Base case**: The size at which the recursive algorithm switches to naive multiplication (default: 45).
- **Time (ms)**: Execution time in milliseconds for each method.

---

## Why Do Multiplication Methods Differ?
The performance differences between the three methods stem from how they utilize the CPU cache and handle memory access patterns. Two key concepts in this context are **Cache-Aware** and **Cache-Oblivious** algorithms, which apply to the blocked and recursive methods, respectively.

1. **Naive Matrix Multiplication**:
   - **How it works**: Uses three nested loops to compute each element of the result matrix `C` by iterating over rows of `A` and columns of `B`.
   - **Performance**: Slowest due to poor cache utilization. Accesses to `B` are column-wise (stride-N), causing frequent cache misses as data is not contiguous in memory. For a 1024 × 1024 matrix, each row/column access (8192 bytes) exceeds typical cache line sizes (64 bytes), leading to inefficient memory use.
   - **Cache impact**: High number of cache misses, especially for large matrices. This method is neither cache-aware nor cache-oblivious—it ignores cache entirely.

2. **Blocked Matrix Multiplication (Cache-Aware)**:
   - **How it works**: Divides matrices into fixed-size blocks (e.g., 32 × 32) and multiplies these blocks, keeping data accesses within small, cache-friendly regions. The block size (`BLOCK_SIZE`) is explicitly tuned to fit within the L1 cache (48 KB).
   - **Cache-Aware Explanation**: This is a **cache-aware** algorithm because it requires knowledge of the cache size to determine an optimal `BLOCK_SIZE`. For example, a 32 × 32 block (8192 bytes) fits well within the 48 KB L1 cache when considering partial loading (three blocks = 24 KB). The algorithm explicitly optimizes for the cache by ensuring data stays in fast memory during computation.
   - **Performance**: Faster than naive because it improves spatial and temporal locality, reducing cache misses significantly. However, its efficiency depends on choosing the right `BLOCK_SIZE`. If too small, loop overhead increases; if too large, blocks exceed the cache, negating benefits.
   - **Cache impact**: Fewer misses due to localized access, but performance is sensitive to hardware-specific tuning.

3. **Recursive Matrix Multiplication (Cache-Oblivious)**:
   - **How it works**: Recursively splits matrices into four quadrants until reaching a base case (e.g., 45 × 45), then uses naive multiplication. Combines results from sub-problems without explicitly referencing cache size.
   - **Cache-Oblivious Explanation**: This is a **cache-oblivious** algorithm because it doesn’t require prior knowledge of cache parameters (like size or line length). The recursive division naturally adapts to any cache hierarchy: as sub-matrices shrink (e.g., from 1024 × 1024 to 45 × 45), they eventually fit into L1 or L2 cache, improving locality without manual tuning. A 45 × 45 sub-matrix (16200 bytes) ensures three sub-matrices (47.46 KB) fit close to the 48 KB L1 cache limit.
   - **Performance**: Often the fastest in this example (774 ms) due to its adaptability. It balances recursion overhead with locality, performing well across different hardware without needing cache-specific adjustments.
   - **Cache impact**: Good locality at smaller sizes, fewer misses than naive, and competitive with blocked. The recursion overhead (function calls) is a trade-off for its flexibility.

### Cache-Aware vs. Cache-Oblivious
- **Cache-Aware (Blocked)**: Explicitly optimizes for a known cache size (e.g., 48 KB L1). It’s faster when tuned correctly (e.g., `BLOCK_SIZE = 32` or 45), but performance drops if the block size doesn’t match the hardware.
- **Cache-Oblivious (Recursive)**: Adapts to any cache size automatically through recursive subdivision. It’s more portable across systems but may incur higher overhead from recursive calls.
- **Key Trade-off**: Blocked offers peak performance with tuning; recursive offers robustness without tuning.

In the example output, recursive (774 ms) outperforms blocked (914 ms) and naive (1294 ms), likely because its cache-oblivious nature better adapts to the 48 KB L1 cache, while blocked’s `BLOCK_SIZE = 32` might not be perfectly optimal. Naive lags due to its lack of cache consideration.

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
Ensure you have CMake and a C++ compiler (e.g., g++) installed. Compile with optimization for accurate performance results:
```bash
cmake -S . -B build
cmake --build build
```

### 3. Run the Program

#### For Windows Users
Example with arguments:
```bash
./build/main.exe --block-size 64 --base-case 32 --n 512
```
Example without arguments (uses default values):
```bash
./build/main.exe
```

#### For Linux/macOS Users
The executable is named `main` instead of `main.exe`. Run it like this:
```bash
./build/main --block-size 64 --base-case 32 --n 512
```
Or without arguments:
```bash
./build/main
```

#### Explanation of Arguments
- `--block-size`: Sets the block size for the blocked multiplication method.
- `--base-case`: Sets the base case size for the recursive multiplication method.
- `--n`: Sets the size of the square matrices (`N × N`).
  
#### Default Values
If no arguments are provided, the program uses:
- `N = 1024`
- `BLOCK_SIZE = 32`
- `BASE_CASE = 45`

#### Why `BASE_CASE = 45`?
The default base case of 45 is chosen based on the L1 cache size and the data type:
- L1 cache size: 48 KB (49152 bytes).
- Data type: `double` (8 bytes per element).
- Three 45 × 45 sub-matrices (A, B, C) = 45 × 45 × 8 × 3 = 48600 bytes (47.46 KB), which fits within the 48 KB L1 cache.
This size ensures that the recursive algorithm’s base case maximizes cache locality, reducing misses while keeping recursion overhead manageable.

#### Why `BLOCK_SIZE = 32`?
The default block size of 32 is a conservative choice:
- A 32 × 32 block = 32 × 32 × 8 = 8192 bytes (8 KB).
- Three blocks = 24 KB, well within 48 KB.
It balances locality with loop overhead, though tuning (e.g., 45 or 64) might yield better results depending on hardware.
