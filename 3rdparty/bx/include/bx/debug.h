/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_DEBUG_H_HEADER_GUARD
#define BX_DEBUG_H_HEADER_GUARD

#include <stdint.h> // uint32_t
#include <stdarg.h> // va_list

namespace bx
{
	class StringView;

	///
	void debugBreak();

	///
	void debugOutput(const char* _out);

	///
	void debugOutput(const StringView& _str);

	///
	void debugPrintfVargs(const char* _format, va_list _argList);

	///
	void debugPrintf(const char* _format, ...);

	///
	void debugPrintfData(const void* _data, uint32_t _size, const char* _format, ...);

	///
	struct WriterI* getDebugOut();

} // namespace bx

#endif // BX_DEBUG_H_HEADER_GUARD
