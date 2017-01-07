/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
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

TEST_CASE("strlncpy", "")
{
	char dst[128];
	size_t num;

	num = bx::strlncpy(dst, 1, "blah");
	REQUIRE(num == 0);

	num = bx::strlncpy(dst, 3, "blah", 3);
	REQUIRE(0 == strcmp(dst, "bl") );
	REQUIRE(num == 2);

	num = bx::strlncpy(dst, sizeof(dst), "blah", 3);
	REQUIRE(0 == strcmp(dst, "bla") );
	REQUIRE(num == 3);

	num = bx::strlncpy(dst, sizeof(dst), "blah");
	REQUIRE(0 == strcmp(dst, "blah") );
	REQUIRE(num == 4);
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

	st.append("test");
	REQUIRE(8 == st.getLength() );

	st.append("test", 2);
	REQUIRE(10 == st.getLength() );

	REQUIRE(0 == strcmp(st.getPtr(), "testtestte") );

	st.clear();
	REQUIRE(0 == st.getLength() );
	REQUIRE(4 == sv.getLength() );

	st.append("test");
	REQUIRE(4 == st.getLength() );

	sv.clear();
	REQUIRE(0 == sv.getLength() );
}
