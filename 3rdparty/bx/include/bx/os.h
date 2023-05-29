/*
 * Copyright 2010-2022 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_OS_H_HEADER_GUARD
#define BX_OS_H_HEADER_GUARD

#include "filepath.h"

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
	void* dlopen(const FilePath& _filePath);

	///
	void dlclose(void* _handle);

	///
	void* dlsym(void* _handle, const StringView& _symbol);

	///
	template<typename ProtoT>
	ProtoT dlsym(void* _handle, const StringView& _symbol);

	///
	bool getEnv(char* _out, uint32_t* _inOutSize, const StringView& _name);

	///
	void setEnv(const StringView& _name, const StringView& _value);

	///
	int chdir(const char* _path);

	///
	void* exec(const char* const* _argv);

	///
	BX_NO_RETURN void exit(int32_t _exitCode);

} // namespace bx

#include "inline/os.inl"

#endif // BX_OS_H_HEADER_GUARD
