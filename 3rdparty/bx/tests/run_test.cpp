/*
 * Copyright 2010-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#define CATCH_CONFIG_RUNNER
#include "test.h"
#include <bx/string.h>

bool testAssertHandler(const bx::Location& _location, const char* _format, va_list _argList)
{
	bx::printf("%s(%d): ", _location.filePath, _location.line);
	bx::vprintf(_format, _argList);
	bx::printf("\n");

	// Throwing exceptions is required for testing asserts being trigged.
	// Use REQUIRE_ASSERTS to test asserts.
	throw std::exception();

	return true;
}

int runAllTests(int _argc, const char* _argv[])
{
	bx::setAssertHandler(testAssertHandler);

	DBG("Compiler: " BX_COMPILER_NAME
		", CPU: " BX_CPU_NAME
		", Architecture: " BX_ARCH_NAME
		", OS: " BX_PLATFORM_NAME
		", CRT: " BX_CRT_NAME
		", Date: " __DATE__
		", Time: " __TIME__
		", C++: " BX_CPP_NAME
		);

	using namespace Catch;

	Session session;

	ConfigData config;
	config.defaultColourMode = BX_PLATFORM_EMSCRIPTEN ? ColourMode::None : ColourMode::PlatformDefault;

	session.useConfigData(config);

	return session.run(_argc, _argv);
}
