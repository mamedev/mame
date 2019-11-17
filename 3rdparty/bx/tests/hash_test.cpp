/*
 * Copyright 2010-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"
#include <bx/hash.h>

void makeCrcTable(uint32_t _poly)
{
	for (uint32_t ii = 0; ii < 256; ++ii)
	{
		uint32_t crc = ii;
		for (uint32_t jj = 0; jj < 8; ++jj)
		{
			if (1 == (crc & 1) )
			{
				crc = (crc >> 1) ^ _poly;
			}
			else
			{
				crc >>= 1;
			}
		}

		printf("0x%08x,%c", crc, 7 == (ii & 7) ? '\n' : ' ');
	}
}

struct HashTest
{
	uint32_t crc32;
	uint32_t adler32;
	const char* input;
};

const HashTest s_hashTest[] =
{
	{ 0,          1,          ""       },
	{ 0xe8b7be43, 0x00620062, "a"      },
	{ 0x9e83486d, 0x012600c4, "ab"     },
	{ 0xc340daab, 0x06060205, "abvgd"  },
	{ 0x07642fe2, 0x020a00d6, "1389"   },
	{ 0x26d75737, 0x04530139, "555333" },
};

TEST_CASE("HashCrc32", "")
{
#if 0
	makeCrcTable(0xedb88320);
	printf("\n");
	makeCrcTable(0x82f63b78);
	printf("\n");
	makeCrcTable(0xeb31d82e);
#endif // 0

	for (uint32_t ii = 0; ii < BX_COUNTOF(s_hashTest); ++ii)
	{
		const HashTest& test = s_hashTest[ii];

		bx::HashCrc32 hash;
		hash.begin();
		hash.add(test.input, bx::strLen(test.input) );
		REQUIRE(test.crc32 == hash.end() );
	}
}

TEST_CASE("HashAdler32", "")
{
	for (uint32_t ii = 0; ii < BX_COUNTOF(s_hashTest); ++ii)
	{
		const HashTest& test = s_hashTest[ii];

		bx::HashAdler32 hash;
		hash.begin();
		hash.add(test.input, bx::strLen(test.input) );
		REQUIRE(test.adler32 == hash.end() );
	}
}
