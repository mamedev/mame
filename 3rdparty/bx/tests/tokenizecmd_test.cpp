/*
 * Copyright 2012-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"
#include <bx/commandline.h>
#include <bx/string.h>

TEST_CASE("commandLine", "")
{
	const char* args[] =
	{
		"-s",
		"--long",
		"--platform",
		"x",
		"--num", "1389",
		"--foo",
		"--", // it should not parse arguments after argument terminator
		"--bar",
	};

	bx::CommandLine cmdLine(BX_COUNTOF(args), args);

	REQUIRE( cmdLine.hasArg("long") );
	REQUIRE( cmdLine.hasArg('s') );

	int32_t num;
	REQUIRE(cmdLine.hasArg(num, '\0', "num") );
	REQUIRE(1389 == num);

	// test argument terminator
	REQUIRE( cmdLine.hasArg("foo") );
	REQUIRE(!cmdLine.hasArg("bar") );

	// non-existing argument
	REQUIRE(!cmdLine.hasArg('x') );
	REQUIRE(!cmdLine.hasArg("preprocess") );
}

static bool test(const char* _input, int32_t _argc, ...)
{
	char buffer[1024];
	uint32_t len = sizeof(buffer);
	char* argv[32];
	int32_t argc;
	bx::tokenizeCommandLine(_input, buffer, len, argc, argv, BX_COUNTOF(argv) );

	if (_argc != argc)
	{
		return false;
	}

	va_list argList;
	va_start(argList, _argc);

	for (int32_t ii = 0; ii < _argc; ++ii)
	{
		const char* arg = va_arg(argList, const char*);
		if (0 != bx::strCmp(argv[ii], arg) )
		{
			return false;
		}
	}

	va_end(argList);

	return true;
}

TEST_CASE("tokenizeCommandLine", "")
{
	REQUIRE(test("      ", 0, NULL) );
	REQUIRE(test("\\",     0, NULL) );

	REQUIRE(test("a b v g d", 5, "a", "b", "v", "g", "d") );

	REQUIRE(test("\"ab\\\"v\" \"\\\\\" g", 3, "ab\"v",    "\\",   "g") );
	REQUIRE(test("a\\\\\\\"b v g",         3, "a\\\"b",   "v",  "g") );
	REQUIRE(test("a\\\\\\\\\"b v\" g d",   3, "a\\\\b v", "g",  "d") );
}
