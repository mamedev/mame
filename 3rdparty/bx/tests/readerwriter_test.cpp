/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#include "test.h"
#include <bx/readerwriter.h>

TEST_CASE("writeLE", "")
{
	bx::SizerWriter writer;

	bx::Error err;

	int32_t total = bx::writeLE(&writer, 1.0f, &err);

	REQUIRE(err.isOk() );
	REQUIRE(total == 4);
}

TEST_CASE("writeBE", "")
{
	bx::SizerWriter writer;

	bx::Error err;

	int32_t total = bx::writeBE(&writer, 1.0f, &err);

	REQUIRE(err.isOk() );
	REQUIRE(total == 4);
}

TEST_CASE("writeRep", "")
{
	uint8_t tmp[1389];
	bx::StaticMemoryBlock mb(tmp, sizeof(tmp) );
	bx::MemoryWriter writer(&mb);

	bx::Error err;

	int32_t total = 0;

	total += bx::writeRep(&writer, 0xfb, BX_COUNTOF(tmp)-1, &err);
	REQUIRE(err.isOk() );
	REQUIRE(BX_COUNTOF(tmp)-1 == total);

	total += bx::writeRep(&writer, 0xfb, 2, &err);
	REQUIRE(!err.isOk() );
	REQUIRE(BX_COUNTOF(tmp) == total);

	for (uint32_t ii = 0; ii < BX_COUNTOF(tmp); ++ii)
	{
		REQUIRE(0xfb == tmp[ii]);
	}
}
