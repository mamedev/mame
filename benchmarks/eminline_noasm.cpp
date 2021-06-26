#include "benchmark/benchmark_api.h"
#include <ctime>
#include "osdcore.h"
#include "osdcomm.h"
#define MAME_NOASM 1
osd_ticks_t osd_ticks(void)
{
	// use the standard library clock function
	return clock();
}
#include "eminline.h"

static void BM_count_leading_zeros_noasm(benchmark::State& state) {
	uint32_t cnt = 0x332533;
	while (state.KeepRunning()) {
		(void)count_leading_zeros_32(cnt);
		cnt++;
	}
}
// Register the function as a benchmark
BENCHMARK(BM_count_leading_zeros_noasm);
