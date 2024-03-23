/*
 * Copyright 2010-2022 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#include "test.h"

TEST_CASE("memSet", "")
{
	char temp[] =  { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0 };

	bx::memSet(temp, 0, 0);
	REQUIRE(temp[0] == 1);

	bx::memSet(temp, 0, 5);
	REQUIRE(temp[0] == 0);
	REQUIRE(temp[1] == 0);
	REQUIRE(temp[2] == 0);
	REQUIRE(temp[3] == 0);
	REQUIRE(temp[4] == 0);
	REQUIRE(temp[5] == 6);
}

TEST_CASE("memMove", "")
{
	const char* original = "xxxxabvgd";
	char str[] = { 'x', 'x', 'x', 'x', 'a', 'b', 'v', 'g', 'd' };

	bx::memMove(&str[4], &str[4], 0);
	REQUIRE(0 == bx::memCmp(str, original, 9) );

	bx::memMove(&str[4], &str[4], 5);
	REQUIRE(0 == bx::memCmp(str, original, 9) );

	bx::memMove(str, &str[4], 5);
	REQUIRE(0 == bx::memCmp(str, "abvgd", 5) );

	bx::memMove(&str[4], str, 5);
	REQUIRE(str[4] == 'a' );

	bx::memSet(str, 'x', 4);
	REQUIRE(0 == bx::memCmp(str, original, 9) );
}

TEST_CASE("scatter/gather", "")
{
	const char* str = "a\0b\0v\0g\0d";

	char tmp0[64];
	bx::gather(tmp0, str, 2, 1, 5);
	REQUIRE(0 == bx::memCmp(tmp0, "abvgd", 5) );

	char tmp1[64];
	bx::scatter(tmp1, 2, tmp0, 1, 5);
	bx::memSet(&tmp1[1], 2, 0, 1, 5);
	REQUIRE(0 == bx::memCmp(tmp1, str, 5) );

}
