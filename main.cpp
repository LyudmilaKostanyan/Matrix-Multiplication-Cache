#include <iostream>
#include <iomanip>
#include <random>
#include <thread>
#include <vector>
#include <algorithm>
#include "kaizen.h"

#ifdef _MSC_VER
#include <malloc.h>
#define aligned_alloc(align, size) _aligned_malloc(size, align)
#define aligned_free(ptr) _aligned_free(ptr)
#else
#include <stdlib.h>
#define aligned_free(ptr) free(ptr)
#endif

constexpr int ALIGNMENT = 64;
int N = 1024;
int BLOCK_SIZE = 32;
int BASE_CASE = 45;

inline double& access(double* mat, int row, int col, int n) {
    return mat[row * n + col];
}

double* allocate_aligned_matrix(int size) {
    return static_cast<double*>(aligned_alloc(ALIGNMENT, size * sizeof(double)));
}

void initializeMatrix(double* mat, int n) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    for (int i = 0; i < n * n; ++i)
        mat[i] = dis(gen);
}

void naiveMatrixMultiply(double* A, double* B, double* C, int n) {
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k)
                sum += access(A, i, k, n) * access(B, k, j, n);
            access(C, i, j, n) = sum;
        }
}

void blockedMatrixMultiply(double* A, double* B, double* C, int n) {
    for (int i = 0; i < n; i += BLOCK_SIZE)
        for (int j = 0; j < n; j += BLOCK_SIZE)
            for (int k = 0; k < n; k += BLOCK_SIZE)
                for (int ii = i; ii < std::min(i + BLOCK_SIZE, n); ++ii)
                    for (int jj = j; jj < std::min(j + BLOCK_SIZE, n); ++jj)
                        for (int kk = k; kk < std::min(k + BLOCK_SIZE, n); ++kk)
                            access(C, ii, jj, n) += access(A, ii, kk, n) * access(B, kk, jj, n);
}

void parallelBlockedMultiply(double* A, double* B, double* C, int n, int num_threads) {
    std::vector<std::thread> threads;

    auto worker = [&](int tid) {
        for (int i = tid * BLOCK_SIZE; i < n; i += BLOCK_SIZE * num_threads)
            for (int j = 0; j < n; j += BLOCK_SIZE)
                for (int k = 0; k < n; k += BLOCK_SIZE)
                    for (int ii = i; ii < std::min(i + BLOCK_SIZE, n); ++ii)
                        for (int jj = j; jj < std::min(j + BLOCK_SIZE, n); ++jj)
                            for (int kk = k; kk < std::min(k + BLOCK_SIZE, n); ++kk)
                                access(C, ii, jj, n) += access(A, ii, kk, n) * access(B, kk, jj, n);
    };

    for (int t = 0; t < num_threads; ++t)
        threads.emplace_back(worker, t);

    for (auto& t : threads)
        t.join();
}

void recursiveMatrixMultiply(double* A, double* B, double* C,
                              int rowA, int colA, int rowB, int colB, int rowC, int colC, int size, int n) {
    if (size <= BASE_CASE) {
        for (int i = 0; i < size; ++i)
            for (int j = 0; j < size; ++j)
                for (int k = 0; k < size; ++k)
                    access(C, rowC + i, colC + j, n) += access(A, rowA + i, colA + k, n) * access(B, rowB + k, colB + j, n);
        return;
    }

    int half = size / 2;

    recursiveMatrixMultiply(A, B, C, rowA, colA, rowB, colB, rowC, colC, half, n);
    recursiveMatrixMultiply(A, B, C, rowA, colA + half, rowB + half, colB, rowC, colC, half, n);

    recursiveMatrixMultiply(A, B, C, rowA, colA, rowB, colB + half, rowC, colC + half, half, n);
    recursiveMatrixMultiply(A, B, C, rowA, colA + half, rowB + half, colB + half, rowC, colC + half, half, n);

    recursiveMatrixMultiply(A, B, C, rowA + half, colA, rowB, colB, rowC + half, colC, half, n);
    recursiveMatrixMultiply(A, B, C, rowA + half, colA + half, rowB + half, colB, rowC + half, colC, half, n);

    recursiveMatrixMultiply(A, B, C, rowA + half, colA, rowB, colB + half, rowC + half, colC + half, half, n);
    recursiveMatrixMultiply(A, B, C, rowA + half, colA + half, rowB + half, colB + half, rowC + half, colC + half, half, n);
}

void parsing_input(int argc, char** argv) {
    zen::cmd_args args(argv, argc);

    if (args.is_present("--block-size")) {
        auto block_size = std::stoi(args.get_options("--block-size")[0]);
        if (block_size > 0)
            BLOCK_SIZE = block_size;
    }
    if (args.is_present("--base-case")) {
        auto base_case = std::stoi(args.get_options("--base-case")[0]);
        if (base_case > 0)
            BASE_CASE = base_case;
    }
    if (args.is_present("--n")) {
        auto n = std::stoi(args.get_options("--n")[0]);
        if (n > 0)
            N = n;
    }
}

int main(int argc, char** argv) {
    parsing_input(argc, argv);
    int num_threads = std::thread::hardware_concurrency();

    double* A = allocate_aligned_matrix(N * N);
    double* B = allocate_aligned_matrix(N * N);
    double* C1 = allocate_aligned_matrix(N * N);
    double* C2 = allocate_aligned_matrix(N * N);
    double* C3 = allocate_aligned_matrix(N * N);
    double* C4 = allocate_aligned_matrix(N * N);

    std::fill(C1, C1 + N * N, 0.0);
    std::fill(C2, C2 + N * N, 0.0);
    std::fill(C3, C3 + N * N, 0.0);
    std::fill(C4, C4 + N * N, 0.0);

    initializeMatrix(A, N);
    initializeMatrix(B, N);

    zen::timer timer;
    timer.start();
    naiveMatrixMultiply(A, B, C1, N);
    timer.stop();
    auto naiveDuration = timer.duration<zen::timer::msec>();

    timer.start();
    blockedMatrixMultiply(A, B, C2, N);
    timer.stop();
    auto blockedDuration = timer.duration<zen::timer::msec>();

    timer.start();
    parallelBlockedMultiply(A, B, C3, N, num_threads);
    timer.stop();
    auto parallelDuration = timer.duration<zen::timer::msec>();

    timer.start();
    recursiveMatrixMultiply(A, B, C4, 0, 0, 0, 0, 0, 0, N, N);
    timer.stop();
    auto recursiveDuration = timer.duration<zen::timer::msec>();

    std::cout << "Parameters:\n";
    std::cout << std::string(35, '-') << "\n";
    std::cout << "Matrix size  | " << N << " x " << N << "\n";
    std::cout << "Block size   | " << BLOCK_SIZE << "\n";
    std::cout << "Base case    | " << BASE_CASE << "\n";
    std::cout << std::string(35, '-') << "\n";

    std::cout << std::left;
    std::cout << std::setw(25) << "Method" << std::setw(15) << "Time (ms)" << "\n";
    std::cout << std::string(40, '-') << "\n";
    std::cout << std::setw(25) << "Naive" << std::setw(15) << naiveDuration.count() << "\n";
    std::cout << std::setw(25) << "Blocked" << std::setw(15) << blockedDuration.count() << "\n";
    std::cout << std::setw(25) << "Parallel Blocked" << std::setw(15) << parallelDuration.count() << "\n";
    std::cout << std::setw(25) << "Recursive" << std::setw(15) << recursiveDuration.count() << "\n";

    aligned_free(A);
    aligned_free(B);
    aligned_free(C1);
    aligned_free(C2);
    aligned_free(C3);
    aligned_free(C4);

    return 0;
}
