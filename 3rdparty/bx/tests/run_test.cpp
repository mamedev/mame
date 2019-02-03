/*
 * Copyright 2010-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#define CATCH_CONFIG_RUNNER
#include "test.h"

int runAllTests(int _argc, const char* _argv[])
{
	DBG("Compiler: " BX_COMPILER_NAME
		", CPU: " BX_CPU_NAME
		", Architecture: " BX_ARCH_NAME
		", OS: " BX_PLATFORM_NAME
		", CRT: " BX_CRT_NAME
		", Date: " __DATE__
		", Time: " __TIME__
		);
	return Catch::Session().run(_argc, _argv);
}
