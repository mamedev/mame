/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"
#include <bx/string.h>
#include <bx/crtimpl.h>
#include <bx/handlealloc.h>

bx::AllocatorI* g_allocator;

TEST_CASE("strnlen", "")
{
	const char* test = "test";

	REQUIRE(0 == bx::strnlen(test, 0) );
	REQUIRE(2 == bx::strnlen(test, 2) );
	REQUIRE(4 == bx::strnlen(test, UINT32_MAX) );
}

TEST_CASE("StringView", "")
{
	bx::StringView sv("test");
	REQUIRE(4 == sv.getLength() );

	bx::CrtAllocator crt;
	g_allocator = &crt;

	typedef bx::StringT<&g_allocator> String;

	String st(sv);
	REQUIRE(4 == st.getLength() );

	st.clear();
	REQUIRE(0 == st.getLength() );
	REQUIRE(4 == sv.getLength() );

	sv.clear();
	REQUIRE(0 == sv.getLength() );
}
