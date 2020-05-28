/*
 * Copyright 2010-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
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
