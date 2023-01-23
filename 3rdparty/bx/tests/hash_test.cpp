/*
 * Copyright 2010-2022 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
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
	uint32_t crc32[bx::HashCrc32::Count];
	uint32_t adler32;
	uint32_t murmur2a;
	const char* input;
};

const HashTest s_hashTest[] =
{
	//  Crc32                               | Adler32   | Murmur2A  | Input
	//  Ieee        Castagnoli  Koopman     |           |           |
	{ { 0,          0,          0          }, 1,          0,          ""       },
	{ { 0xe8b7be43, 0xc1d04330, 0x0da2aa8a }, 0x00620062, 0x0803888b, "a"      },
	{ { 0x9e83486d, 0xe2a22936, 0x31ec935a }, 0x012600c4, 0x618515af, "ab"     },
	{ { 0xc340daab, 0x49e1b6e3, 0x945a1e78 }, 0x06060205, 0x94e3dc4d, "abvgd"  },
	{ { 0x07642fe2, 0x45a04162, 0x3d4bf72d }, 0x020a00d6, 0xe602fc07, "1389"   },
	{ { 0x26d75737, 0xb73d7b80, 0xd524eb40 }, 0x04530139, 0x58d37863, "555333" },
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

		for (uint32_t jj = 0; jj < bx::HashCrc32::Count; ++jj)
		{
			bx::HashCrc32 hash;
			hash.begin(bx::HashCrc32::Enum(jj) );
			hash.add(test.input, bx::strLen(test.input) );
			REQUIRE(test.crc32[jj] == hash.end() );
		}
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

/*-----------------------------------------------------------------------------
// MurmurHash2A, by Austin Appleby
//
// This is a variant of MurmurHash2 modified to use the Merkle-Damgard
// construction. Bulk speed should be identical to Murmur2, small-key speed
// will be 10%-20% slower due to the added overhead at the end of the hash.
//
// This variant fixes a minor issue where null keys were more likely to
// collide with each other than expected, and also makes the function
// more amenable to incremental implementations.
*/

#define mmix(h,k) { k *= m; k ^= k >> r; k *= m; h *= m; h ^= k; }

uint32_t MurmurHash2A(const void * key, int len, uint32_t seed = 0)
{
	const uint32_t m = 0x5bd1e995;
	const int r = 24;
	uint32_t l = len;

	const unsigned char * data = (const unsigned char *)key;

	uint32_t h = seed;

	while(len >= 4)
	{
		uint32_t k = *(uint32_t*)data;

		mmix(h,k);

		data += 4;
		len -= 4;
	}

	uint32_t t = 0;

	switch(len)
	{
	case 3: t ^= data[2] << 16; BX_FALLTHROUGH;
	case 2: t ^= data[1] << 8;  BX_FALLTHROUGH;
	case 1: t ^= data[0];
	};

	mmix(h,t);
	mmix(h,l);

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}

TEST_CASE("HashMurmur2A", "")
{
	uint32_t seed = 0;

	for (uint32_t ii = 0; ii < BX_COUNTOF(s_hashTest); ++ii)
	{
		const HashTest& test = s_hashTest[ii];

		bx::HashMurmur2A hash;
		hash.begin(seed);
		hash.add(test.input, bx::strLen(test.input) );
		REQUIRE(test.murmur2a == hash.end() );

		REQUIRE(test.murmur2a == MurmurHash2A(test.input, bx::strLen(test.input), seed) );
	}
}

TEST_CASE("HashMurmur2A-Separate-Add", "")
{
	bx::HashMurmur2A hash;
	hash.begin();
	hash.add("0123456789");
	hash.add("abvgd012345");
	REQUIRE(MurmurHash2A("0123456789abvgd012345", 21) == hash.end() );
}
