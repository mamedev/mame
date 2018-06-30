/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_OS_H_HEADER_GUARD
#define BX_OS_H_HEADER_GUARD

#include "debug.h"

#if BX_PLATFORM_OSX
#	define BX_DL_EXT "dylib"
#elif BX_PLATFORM_WINDOWS
#	define BX_DL_EXT "dll"
#else
#	define BX_DL_EXT "so"
#endif //

namespace bx
{
	///
	void sleep(uint32_t _ms);

	///
	void yield();

	///
	uint32_t getTid();

	///
	size_t getProcessMemoryUsed();

	///
	void* dlopen(const char* _filePath);

	///
	void dlclose(void* _handle);

	///
	void* dlsym(void* _handle, const char* _symbol);

	///
	bool getenv(const char* _name, char* _out, uint32_t* _inOutSize);

	///
	void setenv(const char* _name, const char* _value);

	///
	void unsetenv(const char* _name);

	///
	int chdir(const char* _path);

	///
	void* exec(const char* const* _argv);

} // namespace bx

#endif // BX_OS_H_HEADER_GUARD
