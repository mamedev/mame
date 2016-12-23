/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#define CATCH_CONFIG_RUNNER
#include "test.h"

static const char* s_argv[] = { "bx.test" };

int runAllTests(int _argc, const char* _argv[])
{
	DBG(BX_COMPILER_NAME " / " BX_CPU_NAME " / " BX_ARCH_NAME " / " BX_PLATFORM_NAME);
	return Catch::Session().run(_argc, _argv);
}

#if BX_PLATFORM_ANDROID
#	include <android/native_activity.h>

void ANativeActivity_onCreate(ANativeActivity*, void*, size_t)
{
	exit(runAllTests(BX_COUNTOF(s_argv), s_argv) );
}
#elif BX_PLATFORM_NACL
#	include <ppapi/c/pp_errors.h>
#	include <ppapi/c/ppp.h>

PP_EXPORT const void* PPP_GetInterface(const char* /*_name*/)
{
	return NULL;
}

PP_EXPORT int32_t PPP_InitializeModule(PP_Module /*_module*/, PPB_GetInterface /*_interface*/)
{
	DBG("PPAPI version: %d", PPAPI_RELEASE);
	runAllTests(BX_COUNTOF(s_argv), s_argv);
	return PP_ERROR_NOINTERFACE;
}

PP_EXPORT void PPP_ShutdownModule()
{
}
#else
int main(int _argc, const char* _argv[])
{
	return runAllTests(_argc, _argv);
}
#endif // BX_PLATFORM
