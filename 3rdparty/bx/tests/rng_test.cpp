/*
 * Copyright 2010-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"

#include <bx/rng.h>
#include <bx/file.h>

const uint32_t kMax = 10<<20;

template<typename Ty>
void testRng(const char* _name, Ty* _rng)
{
	uint32_t histBits[32];
	uint32_t histUint8[256];

	bx::memSet(histBits,  0, sizeof(histBits)  );
	bx::memSet(histUint8, 0, sizeof(histUint8) );

	for (uint32_t ii = 0; ii < kMax; ++ii)
	{
		uint32_t val = _rng->gen();

		for (uint32_t shift = 0; shift < 32; ++shift)
		{
			const uint32_t mask = 1<<shift;
			histBits[shift] += !!(0 != (val & mask) );
		}

		const uint32_t mf000 = (val & 0xff000000)>>24;
		const uint32_t m0f00 = (val & 0x00ff0000)>>16;
		const uint32_t m00f0 = (val & 0x0000ff00)>> 8;
		const uint32_t m000f = (val & 0x000000ff)>> 0;

		histUint8[mf000]++;
		histUint8[m0f00]++;
		histUint8[m00f0]++;
		histUint8[m000f]++;
	}

	bx::WriterI* writer = bx::getNullOut();
	bx::Error err;

	bx::write(writer, &err, "%s\n", _name);

	{
		bx::write(writer, &err, "\tbits histogram:\n");
		uint32_t min = UINT32_MAX;
		uint32_t max = 0;

		for (uint32_t ii = 0; ii < BX_COUNTOF(histBits); ++ii)
		{
			bx::write(writer, &err, "\t\t%3d: %d\n", ii, histBits[ii]);
			min = bx::min(min, histBits[ii]);
			max = bx::max(max, histBits[ii]);
		}

		bx::write(writer, &err, "\tmin: %d, max: %d (diff: %d)\n", min, max, max-min);

		REQUIRE(max-min < 8000);
	}

	{
		bx::write(writer, &err, "\tuint8_t histogram:\n");
		uint32_t min = UINT32_MAX;
		uint32_t max = 0;

		for (uint32_t ii = 0; ii < BX_COUNTOF(histUint8); ++ii)
		{
			bx::write(writer, &err, "\t\t%3d: %d\n", ii, histUint8[ii]);
			min = bx::min(min, histUint8[ii]);
			max = bx::max(max, histUint8[ii]);
		}

		bx::write(writer, &err, "\tmin: %d, max: %d (diff: %d)\n", min, max, max-min);

		REQUIRE(max-min < 8000);
	}
}

TEST_CASE("Rng", "")
{
	bx::RngMwc mwc;
	testRng("RngMwc", &mwc);

	bx::RngShr3 shr3;
	testRng("RngShr3", &shr3);
}
