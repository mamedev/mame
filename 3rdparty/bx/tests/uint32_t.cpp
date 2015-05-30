/*
 * Copyright 2010-2015 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#include "test.h"
#include <bx/uint32_t.h>

TEST(StrideAlign)
{
	CHECK(0  == bx::strideAlign(0, 12) );
	for (uint32_t ii = 0; ii < 12; ++ii)
	{
		CHECK(12 == bx::strideAlign(ii+1, 12) );
	}

	CHECK(0  == bx::strideAlign16(0, 12) );
	for (uint32_t ii = 0; ii < 12; ++ii)
	{
		CHECK(48 == bx::strideAlign16(ii+1, 12) );
	}
}
