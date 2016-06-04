#include "benchmark/benchmark_api.h"
#include "osdcomm.h"
#include "osdcore.h"
#include "eminline.h"
static void BM_count_leading_zeros_native(benchmark::State& state) {
	UINT32 cnt = 0x332533;
	while (state.KeepRunning()) {
		(void)count_leading_zeros(cnt);
		cnt++;
	}
}
// Register the function as a benchmark
BENCHMARK(BM_count_leading_zeros_native);
