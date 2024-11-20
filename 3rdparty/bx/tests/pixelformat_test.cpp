/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#include "test.h"
#include <bx/pixelformat.h>

TEST_CASE("pack/unpack Rgba8", "[pixelformat]")
{
	float rgba[4] = { 0.1f, 0.3f, 0.8f, 0.9f };
	uint32_t encoded;
	bx::packRgba8(&encoded, rgba);

	float decoded[4];
	bx::unpackRgba8(decoded, &encoded);

	REQUIRE(bx::isEqual(rgba, decoded, 4, 0.01f) );
}

TEST_CASE("pack/unpack Rgb9E5F", "[pixelformat]")
{
	float rgba[3] = { 0.1f, 0.3f, 0.89f };
	uint32_t encoded;
	bx::packRgb9E5F(&encoded, rgba);

	float decoded[3];
	bx::unpackRgb9E5F(decoded, &encoded);

	REQUIRE(bx::isEqual(rgba, decoded, BX_COUNTOF(rgba), 0.001f) );
}
