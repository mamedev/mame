/*
 * Copyright 2010-2015 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#ifndef BX_PROCESS_H_HEADER_GUARD
#define BX_PROCESS_H_HEADER_GUARD

#include "string.h"
#include "uint32_t.h"

#if BX_PLATFORM_LINUX
#	include <unistd.h>
#endif // BX_PLATFORM_LINUX

namespace bx
{
	///
	inline void* exec(const char* const* _argv)
	{
#if BX_PLATFORM_LINUX
		pid_t pid = fork();

		if (0 == pid)
		{
			int result = execvp(_argv[0], const_cast<char *const*>(&_argv[1]) );
			BX_UNUSED(result);
			return NULL;
		}

		return (void*)uintptr_t(pid);
#elif BX_PLATFORM_WINDOWS
		STARTUPINFO si;
		memset(&si, 0, sizeof(STARTUPINFO) );
		si.cb = sizeof(STARTUPINFO);

		PROCESS_INFORMATION pi;
		memset(&pi, 0, sizeof(PROCESS_INFORMATION) );

		int32_t total = 0;
		for (uint32_t ii = 0; NULL != _argv[ii]; ++ii)
		{
			total += (int32_t)strlen(_argv[ii]) + 1;
		}

		char* temp = (char*)alloca(total);
		int32_t len = 0;
		for(uint32_t ii = 0; NULL != _argv[ii]; ++ii)
		{
			len += snprintf(&temp[len], bx::uint32_imax(0, total-len)
						, "%s "
						, _argv[ii]
						);
		}

		bool ok = CreateProcessA(_argv[0]
					, temp
					, NULL
					, NULL
					, false
					, 0
					, NULL
					, NULL
					, &si
					, &pi
					);
		if (ok)
		{
			return pi.hProcess;
		}

		return NULL;
#else
		return NULL;
#endif // BX_PLATFORM_LINUX
	}

} // namespace bx

#endif // BX_PROCESS_H_HEADER_GUARD
