#include <iostream>
#include <vector>
#include <random>
#include "kaizen.h"

int N = 1024;
int BLOCK_SIZE = 32;
int BASE_CASE = 45;

void initializeMatrix(auto& mat) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            mat[i][j] = dis(gen);
}

void naiveMatrixMultiply(auto& A, auto& B, auto& C) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            C[i][j] = 0;
            for (int k = 0; k < N; ++k)
                C[i][j] += A[i][k] * B[k][j];
        }
    }
}

void blockedMatrixMultiply(auto& A, auto& B, auto& C) {
    for (int i = 0; i < N; i += BLOCK_SIZE) 
        for (int j = 0; j < N; j += BLOCK_SIZE) 
            for (int k = 0; k < N; k += BLOCK_SIZE) 
                for (int ii = i; ii < std::min(i + BLOCK_SIZE, N); ++ii) 
                    for (int jj = j; jj < std::min(j + BLOCK_SIZE, N); ++jj) 
                        for (int kk = k; kk < std::min(k + BLOCK_SIZE, N); ++kk) 
                            C[ii][jj] += A[ii][kk] * B[kk][jj];
}

void recursiveMatrixMultiply(auto& A, auto& B, auto& C, 
                            int rowA, int colA, int rowB, int colB, int rowC, int colC, int size) {
    if (size <= BASE_CASE) {
        for (int i = 0; i < size; ++i)
            for (int j = 0; j < size; ++j)
                for (int k = 0; k < size; ++k)
                    C[rowC + i][colC + j] += A[rowA + i][colA + k] * B[rowB + k][colB + j];
        return;
    }

    int half = size / 2;

    recursiveMatrixMultiply(A, B, C, rowA, colA, rowB, colB, rowC, colC, half);
    recursiveMatrixMultiply(A, B, C, rowA, colA + half, rowB + half, colB, rowC, colC, half);

    recursiveMatrixMultiply(A, B, C, rowA, colA, rowB, colB + half, rowC, colC + half, half);
    recursiveMatrixMultiply(A, B, C, rowA, colA + half, rowB + half, colB + half, rowC, colC + half, half);

    recursiveMatrixMultiply(A, B, C, rowA + half, colA, rowB, colB, rowC + half, colC, half);
    recursiveMatrixMultiply(A, B, C, rowA + half, colA + half, rowB + half, colB, rowC + half, colC, half);

    recursiveMatrixMultiply(A, B, C, rowA + half, colA, rowB, colB + half, rowC + half, colC + half, half);
    recursiveMatrixMultiply(A, B, C, rowA + half, colA + half, rowB + half, colB + half, rowC + half, colC + half, half);
}

void parsing_input(int argc, char** argv)
{
    zen::cmd_args args(argv, argc);

    if (args.is_present("--block-size"))
    {
        auto block_size = std::stoi(args.get_options("--block-size")[0]);
        if (block_size > 0)
        BLOCK_SIZE = block_size;
    }
    if (args.is_present("--base-case"))
    {
        auto base_case = std::stoi(args.get_options("--base-case")[0]);
        if (base_case > 0)
            BASE_CASE = base_case;
    }
    if (args.is_present("--n"))
    {
        auto n = std::stoi(args.get_options("--n")[0]);
        if (n > 0)
            N = n;
    }
}

int main(int argc, char** argv) {
    parsing_input(argc, argv);

    std::vector<std::vector<double>> A(N, std::vector<double>(N));
    std::vector<std::vector<double>> B(N, std::vector<double>(N));
    std::vector<std::vector<double>> C1(N, std::vector<double>(N, 0));
    std::vector<std::vector<double>> C2(N, std::vector<double>(N, 0));
    std::vector<std::vector<double>> C3(N, std::vector<double>(N, 0));

    initializeMatrix(A);
    initializeMatrix(B);

    zen::timer timer;
    auto naiveDuration = timer.duration<zen::timer::msec>();
    auto blockedDuration = timer.duration<zen::timer::msec>();
    auto recursiveDuration = timer.duration<zen::timer::msec>();

    timer.start();
    naiveMatrixMultiply(A, B, C1);
    timer.stop();
    naiveDuration = timer.duration<zen::timer::msec>();

    timer.start();
    blockedMatrixMultiply(A, B, C2);
    timer.stop();
    blockedDuration = timer.duration<zen::timer::msec>();

    timer.start();
    recursiveMatrixMultiply(A, B, C3, 0, 0, 0, 0, 0, 0, N);
    timer.stop();
    recursiveDuration = timer.duration<zen::timer::msec>();

    std::cout << "Parameters:\n";
    std::cout << std::string(35, '-') << "\n";
    std::cout << "Matrix size  | " << N << " x " << N << "\n";
    std::cout << "Block size   | " << BLOCK_SIZE << "\n";
    std::cout << "Base case    | " << BASE_CASE << "\n";
    std::cout << std::string(35, '-') << "\n";

    std::cout << std::left;
    std::cout << std::setw(20) << "Method" << std::setw(15) << "Time (ms)" << "\n";
    std::cout << std::string(35, '-') << "\n";
    std::cout << std::setw(20) << "Naive" << std::setw(15) << naiveDuration.count() << "\n";
    std::cout << std::setw(20) << "Blocked" << std::setw(15) << blockedDuration.count() << "\n";
    std::cout << std::setw(20) << "Recursive" << std::setw(15) << recursiveDuration.count() << "\n";

    return 0;
}