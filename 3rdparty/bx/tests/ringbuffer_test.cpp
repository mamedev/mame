/*
 * Copyright 2010-2022 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#include "test.h"
#include <bx/ringbuffer.h>

TEST_CASE("RingBufferControl", "")
{
	bx::RingBufferControl control(16);

	REQUIRE(1  == control.reserve(1)  );
	REQUIRE(0  == control.reserve(16, true) );
	REQUIRE(14 == control.reserve(16) );
	REQUIRE(15 == control.commit(15)  );
	REQUIRE(15 == control.available() );
	REQUIRE(15 == control.consume(15) );
	REQUIRE(0  == control.available() );
}
