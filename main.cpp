#include <iostream>
#include <vector>
#include <random>
#include "kaizen.h"

const int N = 1024;

const int BLOCK_SIZE = 32;

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

int main() {
    std::vector<std::vector<double>> A(N, std::vector<double>(N));
    std::vector<std::vector<double>> B(N, std::vector<double>(N));
    std::vector<std::vector<double>> C1(N, std::vector<double>(N, 0));
    std::vector<std::vector<double>> C2(N, std::vector<double>(N, 0));

    initializeMatrix(A);
    initializeMatrix(B);

    zen::timer timer;
    timer.start();
    naiveMatrixMultiply(A, B, C1);
    timer.stop();
    auto naiveDuration = timer.duration<zen::timer::msec>();
    std::cout << "Naive multiplication time: " << naiveDuration.count() << " ms\n";

    timer.start();
    blockedMatrixMultiply(A, B, C2);
    timer.stop();
    auto blockedDuration = timer.duration<zen::timer::msec>();
    std::cout << "Blocked multiplication time: " << blockedDuration.count() << " ms\n";

    return 0;
}