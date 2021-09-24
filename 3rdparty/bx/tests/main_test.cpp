/*
 * Copyright 2010-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "test.h"

int runAllTests(int _argc, const char* _argv[]);

#if BX_PLATFORM_ANDROID
#	include <android/native_activity.h>

static const char* s_argv[] = { "bx.test" };

void ANativeActivity_onCreate(ANativeActivity*, void*, size_t)
{
	exit(runAllTests(BX_COUNTOF(s_argv), s_argv) );
}
#else
int main(int _argc, const char* _argv[])
{
	return runAllTests(_argc, _argv);
}
#endif // BX_PLATFORM
