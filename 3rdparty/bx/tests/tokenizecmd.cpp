/*
 * Copyright 2012-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"
#include <bx/tokenizecmd.h>
#include <bx/commandline.h>
#include <string.h>

TEST(commandLine)
{
	const char* args[] =
	{
		"-s",
		"--long",
	};

	bx::CommandLine cmdLine(BX_COUNTOF(args), args);

	CHECK(cmdLine.hasArg("long") );
	CHECK(cmdLine.hasArg('s') );

	// non-existing argument
	CHECK(!cmdLine.hasArg('x') );
}

TEST(tokenizeCommandLine)
{
#if 0
	const char* input[] =
	{
		"       ",
		"\\",
//		"\"a b c\" d e",
		"\"ab\\\"c\" \"\\\\\" d",
		"a\\\\\\b d\"e f\"g h",
		"a\\\\\\\"b c d",
		"a\\\\\\\\\"b c\" d e",
	};

	const int expected_argc[] =
	{
		0,
		0,
//		3,
		3,
		3,
		3,
		3
	};

	const char* expected_results[] =
	{
		"a b c", "d", "e",
		"ab\"c", "\\", "d",
		"a\\\\\\b", "de fg", "h",
		"a\\\"b", "c", "d",
		"a\\\\b c", "d", "e",
	};

	const char** expected_argv[] =
	{
		NULL,
		NULL,
//		&expected_results[0],
		&expected_results[3],
		&expected_results[6],
		&expected_results[9],
		&expected_results[12],
	};

	for (uint32_t ii = 0; ii < BX_COUNTOF(exptected_argv); ++ii)
	{
		printf("x\n");
		char commandLine[1024];
		uint32_t size = BX_COUNTOF(commandLine);
		char* argv[50];
		int32_t argc;
		bx::tokenizeCommandLine(input[ii], commandLine, size, argc, argv, BX_COUNTOF(argv) );
		printf("\n%d (%d): %s %s\n", ii, argc, input[ii], expected_argc[ii] == argc ? "" : "FAILED!");
		for (uint32_t jj = 0; jj < argc; ++jj)
		{
			printf("\t%d: {%s} %s\n"
					, jj
					, argv[jj]
					, jj < argc ? (0==strcmp(argv[jj], expected_argv[ii][jj]) ? "" : "FAILED!") : "FAILED!"
					);
		}
	}
#endif // 0
}
