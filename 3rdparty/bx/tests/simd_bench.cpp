/*
 * Copyright 2010-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include <bx/allocator.h>
#include <bx/rng.h>
#include <bx/simd_t.h>
#include <bx/timer.h>

#include <stdio.h>

static void flushCache()
{
	static uint32_t length = 1 << 26;
	static uint8_t* input  = new uint8_t[length];
	static uint8_t* output = new uint8_t[length];
	bx::memCopy(output, input, length);
}

typedef bx::simd128_t (*SimdRsqrtFn)(bx::simd128_t _a);

template<SimdRsqrtFn simdRsqrtFn>
void simd_rsqrt_bench(bx::simd128_t* _dst, bx::simd128_t* _src, uint32_t _numVertices)
{
	for (uint32_t ii = 0, num = _numVertices/4; ii < num; ++ii)
	{
		bx::simd128_t* ptr = &_src[ii*4];
		bx::simd128_t tmp0 = bx::simd_ld(ptr + 0);
		bx::simd128_t tmp1 = bx::simd_ld(ptr + 1);
		bx::simd128_t tmp2 = bx::simd_ld(ptr + 2);
		bx::simd128_t tmp3 = bx::simd_ld(ptr + 3);
		bx::simd128_t rsqrt0 = simdRsqrtFn(tmp0);
		bx::simd128_t rsqrt1 = simdRsqrtFn(tmp1);
		bx::simd128_t rsqrt2 = simdRsqrtFn(tmp2);
		bx::simd128_t rsqrt3 = simdRsqrtFn(tmp3);

		ptr = &_dst[ii*4];
		bx::simd_st(ptr + 0, rsqrt0);
		bx::simd_st(ptr + 1, rsqrt1);
		bx::simd_st(ptr + 2, rsqrt2);
		bx::simd_st(ptr + 3, rsqrt3);
	}
}

void simd_bench_pass(bx::simd128_t* _dst, bx::simd128_t* _src, uint32_t _numVertices)
{
	const uint32_t numIterations = 10;

	{
		int64_t elapsed = 0;
		for (uint32_t test = 0; test < numIterations; ++test)
		{
			flushCache();
			elapsed += -bx::getHPCounter();
			simd_rsqrt_bench<bx::simd_rsqrt_est>(_dst, _src, _numVertices);
			elapsed += bx::getHPCounter();
		}
		printf("    simd_rsqrt_est: %15f\n", double(elapsed) );
	}

	{
		int64_t elapsed = 0;
		for (uint32_t test = 0; test < numIterations; ++test)
		{
			flushCache();
			elapsed += -bx::getHPCounter();
			simd_rsqrt_bench<bx::simd_rsqrt_nr>(_dst, _src, _numVertices);
			elapsed += bx::getHPCounter();
		}
		printf("     simd_rsqrt_nr: %15f\n", double(elapsed) );
	}

	{
		int64_t elapsed = 0;
		for (uint32_t test = 0; test < numIterations; ++test)
		{
			flushCache();
			elapsed += -bx::getHPCounter();
			simd_rsqrt_bench<bx::simd_rsqrt_carmack>(_dst, _src, _numVertices);
			elapsed += bx::getHPCounter();
		}
		printf("simd_rsqrt_carmack: %15f\n", double(elapsed) );
	}

	{
		int64_t elapsed = 0;
		for (uint32_t test = 0; test < numIterations; ++test)
		{
			flushCache();
			elapsed += -bx::getHPCounter();
			simd_rsqrt_bench<bx::simd_rsqrt>(_dst, _src, _numVertices);
			elapsed += bx::getHPCounter();
		}
		printf("        simd_rsqrt: %15f\n", double(elapsed) );
	}
}

void simd_bench()
{
	bx::DefaultAllocator allocator;
	bx::RngMwc rng;

	const uint32_t numVertices = 1024*1024;

	uint8_t* data = (uint8_t*)BX_ALIGNED_ALLOC(&allocator, 2*numVertices*sizeof(bx::simd128_t), 16);
	bx::simd128_t* src = (bx::simd128_t*)data;
	bx::simd128_t* dst = &src[numVertices];

	printf("\n -- positive & negative --\n");
	for (uint32_t ii = 0; ii < numVertices; ++ii)
	{
		float* ptr = (float*)&src[ii];
		bx::store(ptr, bx::randUnitSphere(&rng) );
		ptr[3] = 1.0f;
	}

	simd_bench_pass(dst, src, numVertices);

	printf("\n -- positive only --\n");
	for (uint32_t ii = 0; ii < numVertices; ++ii)
	{
		float* ptr = (float*)&src[ii];
		ptr[0] = bx::abs(ptr[0]);
		ptr[1] = bx::abs(ptr[1]);
		ptr[2] = bx::abs(ptr[2]);
		ptr[3] = bx::abs(ptr[3]);
	}

	simd_bench_pass(dst, src, numVertices);

	BX_ALIGNED_FREE(&allocator, data, 16);
}
