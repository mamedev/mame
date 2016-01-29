#include "benchmark/benchmark_api.h"

void BM_basic(benchmark::State& state) {
  while (state.KeepRunning()) {
  }
}
BENCHMARK(BM_basic);
BENCHMARK(BM_basic)->Arg(42);
BENCHMARK(BM_basic)->Range(1, 8);
BENCHMARK(BM_basic)->DenseRange(10, 15);
BENCHMARK(BM_basic)->ArgPair(42, 42);
BENCHMARK(BM_basic)->RangePair(64, 512, 64, 512);
BENCHMARK(BM_basic)->MinTime(0.7);
BENCHMARK(BM_basic)->UseRealTime();
BENCHMARK(BM_basic)->ThreadRange(2, 4);
BENCHMARK(BM_basic)->ThreadPerCpu();

void CustomArgs(benchmark::internal::Benchmark* b) {
  for (int i = 0; i < 10; ++i) {
    b->Arg(i);
  }
}

BENCHMARK(BM_basic)->Apply(CustomArgs);

BENCHMARK_MAIN()
