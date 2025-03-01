#include <benchmark/benchmark.h>
#include <algorithm>
#include <array>
#include <execution>
#include <numeric>

static void sequential(benchmark::State& state) {
    std::vector<int> arr(1 << 30);
    // Code inside this loop is measured repeatedly
    for (auto _ : state) {
        auto sum = std::reduce(std::begin(arr), std::end(arr), 0, std::plus<>{});
        BENCHMARK_DONT_OPTIMIZE(sum);
    }
}
BENCHMARK(sequential);

static void paralle(benchmark::State& state) {
    std::vector<int> arr(1 << 30);
    for (auto _ : state) {
        auto sum = std::reduce(std::execution::par_unseq, std::begin(arr), std::end(arr), 0,
                               std::plus<>{});
        BENCHMARK_DONT_OPTIMIZE(sum);
    }
}
BENCHMARK(paralle);

BENCHMARK_MAIN();