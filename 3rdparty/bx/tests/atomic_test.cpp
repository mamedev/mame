/*
 * Copyright 2010-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"
#include <bx/cpu.h>

TEST_CASE("atomic", "")
{
	uint32_t test = 1337;
	uint32_t fetched;

	fetched = bx::atomicFetchAndAdd(&test, 52u);
	REQUIRE(fetched == 1337);
	REQUIRE(test == 1389);

	fetched = bx::atomicAddAndFetch(&test, 64u);
	REQUIRE(fetched == 1453);
	REQUIRE(test == 1453);

	fetched = bx::atomicFetchAndSub(&test, 64u);
	REQUIRE(fetched == 1453);
	REQUIRE(test == 1389);

	fetched = bx::atomicSubAndFetch(&test, 52u);
	REQUIRE(fetched == 1337);
	REQUIRE(test == 1337);

	fetched = bx::atomicFetchAndAddsat(&test, 52u, 1453u);
	REQUIRE(fetched == 1337);
	REQUIRE(test == 1389);

	fetched = bx::atomicFetchAndAddsat(&test, 1000u, 1453u);
	REQUIRE(fetched == 1389);
	REQUIRE(test == 1453);

	fetched = bx::atomicFetchAndSubsat(&test, 64u, 1337u);
	REQUIRE(fetched == 1453);
	REQUIRE(test == 1389);

	fetched = bx::atomicFetchAndSubsat(&test, 1000u, 1337u);
	REQUIRE(fetched == 1389);
	REQUIRE(test == 1337);

}
