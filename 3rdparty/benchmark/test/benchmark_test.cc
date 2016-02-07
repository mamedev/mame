#include "benchmark/benchmark.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>

#include <cstdlib>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#if defined(__GNUC__)
# define BENCHMARK_NOINLINE __attribute__((noinline))
#else
# define BENCHMARK_NOINLINE
#endif

namespace {

int BENCHMARK_NOINLINE Factorial(uint32_t n) {
  return (n == 1) ? 1 : n * Factorial(n - 1);
}

double CalculatePi(int depth) {
  double pi = 0.0;
  for (int i = 0; i < depth; ++i) {
    double numerator = static_cast<double>(((i % 2) * 2) - 1);
    double denominator = static_cast<double>((2 * i) - 1);
    pi += numerator / denominator;
  }
  return (pi - 1.0) * 4;
}

std::set<int> ConstructRandomSet(int size) {
  std::set<int> s;
  for (int i = 0; i < size; ++i)
    s.insert(i);
  return s;
}

std::mutex test_vector_mu;
std::vector<int>* test_vector = nullptr;

}  // end namespace

static void BM_Factorial(benchmark::State& state) {
  int fac_42 = 0;
  while (state.KeepRunning())
    fac_42 = Factorial(8);
  // Prevent compiler optimizations
  std::stringstream ss;
  ss << fac_42;
  state.SetLabel(ss.str());
}
BENCHMARK(BM_Factorial);
BENCHMARK(BM_Factorial)->UseRealTime();

static void BM_CalculatePiRange(benchmark::State& state) {
  double pi = 0.0;
  while (state.KeepRunning())
    pi = CalculatePi(state.range_x());
  std::stringstream ss;
  ss << pi;
  state.SetLabel(ss.str());
}
BENCHMARK_RANGE(BM_CalculatePiRange, 1, 1024 * 1024);

static void BM_CalculatePi(benchmark::State& state) {
  static const int depth = 1024;
  while (state.KeepRunning()) {
    benchmark::DoNotOptimize(CalculatePi(depth));
  }
}
BENCHMARK(BM_CalculatePi)->Threads(8);
BENCHMARK(BM_CalculatePi)->ThreadRange(1, 32);
BENCHMARK(BM_CalculatePi)->ThreadPerCpu();

static void BM_SetInsert(benchmark::State& state) {
  while (state.KeepRunning()) {
    state.PauseTiming();
    std::set<int> data = ConstructRandomSet(state.range_x());
    state.ResumeTiming();
    for (int j = 0; j < state.range_y(); ++j)
      data.insert(rand());
  }
  state.SetItemsProcessed(state.iterations() * state.range_y());
  state.SetBytesProcessed(state.iterations() * state.range_y() * sizeof(int));
}
BENCHMARK(BM_SetInsert)->RangePair(1<<10,8<<10, 1,10);

template<typename Container, typename ValueType = typename Container::value_type>
static void BM_Sequential(benchmark::State& state) {
  ValueType v = 42;
  while (state.KeepRunning()) {
    Container c;
    for (int i = state.range_x(); --i; )
      c.push_back(v);
  }
  const size_t items_processed = state.iterations() * state.range_x();
  state.SetItemsProcessed(items_processed);
  state.SetBytesProcessed(items_processed * sizeof(v));
}
BENCHMARK_TEMPLATE2(BM_Sequential, std::vector<int>, int)->Range(1 << 0, 1 << 10);
BENCHMARK_TEMPLATE(BM_Sequential, std::list<int>)->Range(1 << 0, 1 << 10);
// Test the variadic version of BENCHMARK_TEMPLATE in C++11 and beyond.
#if __cplusplus >= 201103L
BENCHMARK_TEMPLATE(BM_Sequential, std::vector<int>, int)->Arg(512);
#endif

static void BM_StringCompare(benchmark::State& state) {
  std::string s1(state.range_x(), '-');
  std::string s2(state.range_x(), '-');
  while (state.KeepRunning())
    benchmark::DoNotOptimize(s1.compare(s2));
}
BENCHMARK(BM_StringCompare)->Range(1, 1<<20);

static void BM_SetupTeardown(benchmark::State& state) {
  if (state.thread_index == 0) {
    // No need to lock test_vector_mu here as this is running single-threaded.
    test_vector = new std::vector<int>();
  }
  int i = 0;
  while (state.KeepRunning()) {
    std::lock_guard<std::mutex> l(test_vector_mu);
    if (i%2 == 0)
      test_vector->push_back(i);
    else
      test_vector->pop_back();
    ++i;
  }
  if (state.thread_index == 0) {
    delete test_vector;
  }
}
BENCHMARK(BM_SetupTeardown)->ThreadPerCpu();

static void BM_LongTest(benchmark::State& state) {
  double tracker = 0.0;
  while (state.KeepRunning()) {
    for (int i = 0; i < state.range_x(); ++i)
      benchmark::DoNotOptimize(tracker += i);
  }
}
BENCHMARK(BM_LongTest)->Range(1<<16,1<<28);

BENCHMARK_MAIN()

