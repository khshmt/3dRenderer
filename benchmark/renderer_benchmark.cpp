#include <benchmark/benchmark.h>
#include <algorithm>
#include <array>
#include <execution>
#include <numeric>
#include <immintrin.h>
#include "renderer.hpp"
#include "version2/vectorclass.h"

Renderer renderer;

static void SIMDVectorAddition(benchmark::State& state) {
    Vec32uc a(10, 11, 12, 13, 10, 11, 12, 13, 10, 11, 12, 13, 10, 11, 12, 13, 10, 11, 12, 13, 10,
              11, 12, 13, 10, 11, 12, 13, 10, 11, 12, 13);
    Vec32uc b(10, 11, 12, 13, 10, 11, 12, 13, 10, 11, 12, 13, 10, 11, 12, 13, 10, 11, 12, 13, 10,
              11, 12, 13, 10, 11, 12, 13, 10, 11, 12, 13);
    for (auto _ : state) {
        Vec32uc c = a + b;  // Vector addition using VectorClass
    }
}
BENCHMARK(SIMDVectorAddition);

static void vectorAddition(benchmark::State& state) {
    std::vector<int> a{10, 11, 12, 13, 10, 11, 12, 13, 10, 11, 12, 13, 10, 11, 12, 13,
                       10, 11, 12, 13, 10, 11, 12, 13, 10, 11, 12, 13, 10, 11, 12, 13};
    std::vector<int> b{10, 11, 12, 13, 10, 11, 12, 13, 10, 11, 12, 13, 10, 11, 12, 13, 10, 11, 12,
                       13, 10, 11, 12, 13, 10, 11, 12, 13, 10, 11, 12, 13};
    for (auto _ : state) {
        std::vector<int> c;
        for (auto i{0}; i < a.size(); ++i) {
            c.push_back(a[i] + b[i]);  // Simple vector addition
        }
    }
}
BENCHMARK(vectorAddition);

BENCHMARK_MAIN();