/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#include <bx/debug.h>
#include <bx/string.h>       // isPrint
#include <bx/readerwriter.h> // WriterI
#include <inttypes.h>        // PRIx*

#ifndef BX_CONFIG_CALLSTACK_USE_LIB_BACKTRACE
#	define BX_CONFIG_CALLSTACK_USE_LIB_BACKTRACE 0
#elif BX_CONFIG_CALLSTACK_USE_LIB_BACKTRACE
#	if !BX_PLATFORM_LINUX || !BX_COMPILER_GCC
#		error "libbackrace is only supported on GCC/Linux."
#	endif // BX_PLATFORM_LINUX && BX_COMPILER_GCC
#endif // BX_CONFIG_CALLSTACK_USE_LIB_BACKTRACE

#if BX_CONFIG_CALLSTACK_USE_LIB_BACKTRACE
#	include <execinfo.h>  // backtrace
#	include <backtrace.h> // backtrace_syminfo
#	include <cxxabi.h>    // abi::__cxa_demangle
#endif // BX_CONFIG_CALLSTACK_*

#if BX_CRT_NONE
#	include <bx/crt0.h>
#elif BX_PLATFORM_ANDROID
#	include <android/log.h>
#elif  BX_PLATFORM_WINDOWS \
	|| BX_PLATFORM_WINRT   \
	|| BX_PLATFORM_XBOXONE
extern "C" __declspec(dllimport) void __stdcall OutputDebugStringA(const char* _str);
#elif BX_PLATFORM_IOS || BX_PLATFORM_OSX || BX_PLATFORM_VISIONOS
#	if defined(__OBJC__)
#		import <Foundation/NSObjCRuntime.h>
#	else
#		include <CoreFoundation/CFString.h>
extern "C" void NSLog(CFStringRef _format, ...);
#	endif // defined(__OBJC__)
#elif BX_PLATFORM_EMSCRIPTEN
#	include <emscripten/emscripten.h>
#else
#	include <stdio.h> // fputs, fflush
#endif // BX_PLATFORM_WINDOWS

namespace bx
{
	void debugBreak()
	{
#if BX_COMPILER_MSVC
		__debugbreak();
#elif BX_CPU_ARM
		__builtin_trap();
//		asm("bkpt 0");
#elif BX_CPU_X86 && (BX_COMPILER_GCC || BX_COMPILER_CLANG)
		// NaCl doesn't like int 3:
		// NativeClient: NaCl module load failed: Validation failure. File violates Native Client safety rules.
		__asm__ ("int $3");
#elif BX_PLATFORM_EMSCRIPTEN
		emscripten_log(0
			| EM_LOG_CONSOLE
			| EM_LOG_ERROR
			| EM_LOG_C_STACK
			| EM_LOG_JS_STACK
			, "debugBreak!"
			);
		// Doing emscripten_debugger() disables asm.js validation due to an emscripten bug
		//emscripten_debugger();
		EM_ASM({ debugger; });
#else // cross platform implementation
		int* int3 = (int*)3L;
		*int3 = 3;
#endif // BX
	}

	void debugOutput(const char* _out)
	{
#if BX_CRT_NONE
		crt0::debugOutput(_out);
#elif BX_PLATFORM_ANDROID
#	ifndef BX_ANDROID_LOG_TAG
#		define BX_ANDROID_LOG_TAG ""
#	endif // BX_ANDROID_LOG_TAG
		__android_log_write(ANDROID_LOG_DEBUG, BX_ANDROID_LOG_TAG, _out);
#elif  BX_PLATFORM_WINDOWS \
	|| BX_PLATFORM_WINRT   \
	|| BX_PLATFORM_XBOXONE
		OutputDebugStringA(_out);
#elif  BX_PLATFORM_IOS \
	|| BX_PLATFORM_OSX   \
	|| BX_PLATFORM_VISIONOS
#	if defined(__OBJC__)
		NSLog(@"%s", _out);
#	else
		NSLog(__CFStringMakeConstantString("%s"), _out);
#	endif // defined(__OBJC__)
#elif BX_PLATFORM_EMSCRIPTEN
		emscripten_log(EM_LOG_CONSOLE, "%s", _out);
#else
		fputs(_out, stdout);
		fflush(stdout);
#endif // BX_PLATFORM_
	}

	void debugOutput(const StringView& _str)
	{
#if BX_CRT_NONE
		crt0::debugOutput(_str);
#else
		const char* data = _str.getPtr();
		int32_t size = _str.getLength();

		char temp[4096];
		while (0 != size)
		{
			uint32_t len = uint32_min(sizeof(temp)-1, size);
			memCopy(temp, data, len);
			temp[len] = '\0';
			data += len;
			size -= len;
			debugOutput(temp);
		}
#endif // BX_CRT_NONE
	}

	void debugPrintfVargs(const char* _format, va_list _argList)
	{
		char temp[8192];
		char* out = temp;
		int32_t len = vsnprintf(out, sizeof(temp), _format, _argList);
		if ( (int32_t)sizeof(temp) < len)
		{
			out = (char*)alloca(len+1);
			len = vsnprintf(out, len, _format, _argList);
		}
		out[len] = '\0';
		debugOutput(out);
	}

	void debugPrintf(const char* _format, ...)
	{
		va_list argList;
		va_start(argList, _format);
		debugPrintfVargs(_format, argList);
		va_end(argList);
	}

#define DBG_ADDRESS "%" PRIxPTR

	void debugPrintfData(const void* _data, uint32_t _size, const char* _format, ...)
	{
#define HEX_DUMP_WIDTH 16
#define HEX_DUMP_SPACE_WIDTH 48
#define HEX_DUMP_FORMAT "%-" BX_STRINGIZE(HEX_DUMP_SPACE_WIDTH) "." BX_STRINGIZE(HEX_DUMP_SPACE_WIDTH) "s"

		va_list argList;
		va_start(argList, _format);
		debugPrintfVargs(_format, argList);
		va_end(argList);

		debugPrintf("\ndata: " DBG_ADDRESS ", size: %d\n", _data, _size);

		if (NULL != _data)
		{
			const uint8_t* data = (const uint8_t*)_data;
			char hex[HEX_DUMP_WIDTH*3+1];
			char ascii[HEX_DUMP_WIDTH+1];
			uint32_t hexPos = 0;
			uint32_t asciiPos = 0;
			for (uint32_t ii = 0; ii < _size; ++ii)
			{
				snprintf(&hex[hexPos], sizeof(hex)-hexPos, "%02x ", data[asciiPos]);
				hexPos += 3;

				ascii[asciiPos] = isPrint(data[asciiPos]) ? data[asciiPos] : '.';
				asciiPos++;

				if (HEX_DUMP_WIDTH == asciiPos)
				{
					ascii[asciiPos] = '\0';
					debugPrintf("\t" DBG_ADDRESS "\t" HEX_DUMP_FORMAT "\t%s\n", data, hex, ascii);
					data += asciiPos;
					hexPos   = 0;
					asciiPos = 0;
				}
			}

			if (0 != asciiPos)
			{
				ascii[asciiPos] = '\0';
				debugPrintf("\t" DBG_ADDRESS "\t" HEX_DUMP_FORMAT "\t%s\n", data, hex, ascii);
			}
		}

#undef HEX_DUMP_WIDTH
#undef HEX_DUMP_SPACE_WIDTH
#undef HEX_DUMP_FORMAT
	}

	class DebugWriter : public WriterI
	{
		virtual int32_t write(const void* _data, int32_t _size, Error* _err) override
		{
			BX_UNUSED(_err);
			debugOutput(StringView( (const char*)_data, _size) );
			return _size;
		}
	};

	WriterI* getDebugOut()
	{
		static DebugWriter s_debugOut;
		return &s_debugOut;
	}

#if BX_CONFIG_CALLSTACK_USE_LIB_BACKTRACE
	uint32_t getCallStack(uint32_t _skip, uint32_t _max, uintptr_t* _outStack)
	{
		const uint32_t max = _skip+_max+1;
		void** tmp = (void**)alloca(sizeof(uintptr_t)*max);

		const uint32_t numFull = backtrace(tmp, max);
		const uint32_t skip    = min(_skip + 1 /* skip self */, numFull);
		const uint32_t num     = numFull - skip;

		memCopy(_outStack, tmp + skip, sizeof(uintptr_t)*num);

		return num;
	}

	struct StackTraceContext
	{
		StackTraceContext()
		{
			state = backtrace_create_state(NULL, 0, NULL, NULL);
		}

		struct backtrace_state* state;
	};

	static StackTraceContext s_stCtx;

	struct CallbackData
	{
		StringView resolvedName;
		StringView fileName;
		int32_t line;
	};

	static void backtraceSymInfoCb(void* _data, uintptr_t _pc, const char* _symName, uintptr_t _symVal, uintptr_t _symSize)
	{
		BX_UNUSED(_pc, _symVal);

		CallbackData* cbData = (CallbackData*)_data;
		cbData->resolvedName.set(_symName, _symSize);
	}

	static int backtraceFullCb(void* _data, uintptr_t _pc, const char* _fileName, int32_t _lineNo, const char* _function)
	{
		BX_UNUSED(_pc, _function);

		CallbackData* cbData = (CallbackData*)_data;
		if (NULL == _fileName)
		{
			cbData->fileName.set("<Unknown?>");
			cbData->line = -1;
		}
		else
		{
			cbData->fileName.set(_fileName);
			cbData->line = _lineNo;
		}

		return 1;
	}

	int32_t writeCallstack(WriterI* _writer, uintptr_t* _stack, uint32_t _num, Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		char   demangleBuf[4096];
		size_t demangleLen = BX_COUNTOF(demangleBuf);

		int32_t total = write(_writer, _err, "Callstack (%d):\n", _num);

		CallbackData cbData;

		for (uint32_t ii = 0; ii < _num && _err->isOk(); ++ii)
		{
			backtrace_pcinfo(s_stCtx.state, _stack[ii], backtraceFullCb, NULL, &cbData);

			StringView demangledName;

			if (1 == backtrace_syminfo(s_stCtx.state, _stack[ii], backtraceSymInfoCb, NULL, &cbData) )
			{
				demangleLen = BX_COUNTOF(demangleBuf);
				int32_t demangleStatus;
				abi::__cxa_demangle(cbData.resolvedName.getPtr(), demangleBuf, &demangleLen, &demangleStatus);

				if (0 == demangleStatus)
				{
					demangledName.set(demangleBuf, demangleLen);
				}
				else
				{
					demangledName = cbData.resolvedName;
				}
			}
			else
			{
				demangledName = "???";
			}

			constexpr uint32_t width = 40;
			const StringView fn = strTail(cbData.fileName, width);

			total += write(_writer, _err
				, "\t%2d: %-*S % 5d: %p %S\n"
				, ii
				, width
				, &fn
				, cbData.line
				, _stack[ii]
				, &demangledName
				);

			if (0 == strCmp(demangledName, "main", 4) )
			{
				if (0 != _num-1-ii)
				{
					total += write(_writer, _err
						, "\t... %d more stack frames below 'main'.\n"
						, _num-1-ii
						);
				}
				break;
			}
		}

		return total;
	}

#else

	uint32_t getCallStack(uint32_t _skip, uint32_t _max, uintptr_t* _outStack)
	{
		BX_UNUSED(_skip, _max, _outStack);
		return 0;
	}

	int32_t writeCallstack(WriterI* _writer, uintptr_t* _stack, uint32_t _num, Error* _err)
	{
		BX_UNUSED(_writer, _stack, _num, _err);
		return 0;
	}

#endif // BX_CONFIG_CALLSTACK_*

	void debugOutputCallstack(uint32_t _skip)
	{
		uintptr_t stack[32];
		const uint32_t num = getCallStack(_skip + 1 /* skip self */, BX_COUNTOF(stack), stack);
		writeCallstack(getDebugOut(), stack, num, ErrorIgnore{});
	}

} // namespace bx
