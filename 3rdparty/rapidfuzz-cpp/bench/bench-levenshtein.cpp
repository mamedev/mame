#include <benchmark/benchmark.h>
#include <rapidfuzz/string_metric.hpp>
#include <rapidfuzz/details/string_view.hpp>
#include <string>
#include <vector>

namespace string_metric = rapidfuzz::string_metric;

// Define another benchmark
static void BM_LevWeightedDist1(benchmark::State &state) {
  rapidfuzz::string_view a = "aaaaa aaaaa";
  for (auto _ : state) {
    benchmark::DoNotOptimize(string_metric::levenshtein(a, a));
  }
  state.SetLabel("Similar Strings");
}

static void BM_LevWeightedDist2(benchmark::State &state) {
  rapidfuzz::string_view a = "aaaaa aaaaa";
  rapidfuzz::string_view b = "bbbbb bbbbb";
  for (auto _ : state) {
    benchmark::DoNotOptimize(string_metric::levenshtein(a, b));
  }
  state.SetLabel("Different Strings");
}

static void BM_LevNormWeightedDist1(benchmark::State &state) {
  rapidfuzz::string_view a = "aaaaa aaaaa";
  for (auto _ : state) {
    benchmark::DoNotOptimize(string_metric::normalized_levenshtein(a, a));
  }
  state.SetLabel("Similar Strings");
}

static void BM_LevNormWeightedDist2(benchmark::State &state) {
  rapidfuzz::string_view a = "aaaaa aaaaa";
  rapidfuzz::string_view b = "bbbbb bbbbb";
  for (auto _ : state) {
    benchmark::DoNotOptimize(string_metric::normalized_levenshtein(a, b));
  }
  state.SetLabel("Different Strings");
}


BENCHMARK(BM_LevWeightedDist1);
BENCHMARK(BM_LevWeightedDist2);

BENCHMARK(BM_LevNormWeightedDist1);
BENCHMARK(BM_LevNormWeightedDist2);


BENCHMARK_MAIN();