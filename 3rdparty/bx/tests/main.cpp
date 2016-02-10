/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

/*
 * Copyright 2012 Matthew Endsley
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "test.h"

int runAllTests()
{
	DBG(BX_COMPILER_NAME " / " BX_CPU_NAME " / " BX_ARCH_NAME " / " BX_PLATFORM_NAME);
	return UnitTest::RunAllTests();
}

#if BX_PLATFORM_ANDROID
#	include <android/native_activity.h>

void ANativeActivity_onCreate(ANativeActivity*, void*, size_t)
{
	exit(runAllTests() );
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
	runAllTests();
	return PP_ERROR_NOINTERFACE;
}

PP_EXPORT void PPP_ShutdownModule()
{
}
#else
int main()
{
	return runAllTests();
}
#endif // BX_PLATFORM
