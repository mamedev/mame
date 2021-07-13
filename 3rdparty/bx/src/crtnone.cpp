/*
 * Copyright 2010-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "bx_p.h"
#include <bx/debug.h>
#include <bx/file.h>
#include <bx/math.h>
#include <bx/sort.h>
#include <bx/timer.h>

#if BX_CRT_NONE

#include "crt0.h"

#define NOT_IMPLEMENTED() \
	{ bx::debugPrintf("crtnone: %s not implemented\n", BX_FUNCTION); abort(); }

extern "C" void* memcpy(void* _dst, const void* _src, size_t _numBytes)
{
	bx::memCopy(_dst, _src, _numBytes);
	return _dst;
}

extern "C" void* memmove(void* _dst, const void* _src, size_t _numBytes)
{
	bx::memMove(_dst, _src, _numBytes);
	return _dst;
}

extern "C" void* memset(void* _dst, int _ch, size_t _numBytes)
{
	bx::memSet(_dst, uint8_t(_ch), _numBytes);
	return _dst;
}

#if !BX_PLATFORM_NONE

typedef int64_t off64_t;
typedef int32_t pid_t;

extern "C" int32_t memcmp(const void* _lhs, const void* _rhs, size_t _numBytes)
{
	return bx::memCmp(_lhs, _rhs, _numBytes);
}

extern "C" size_t strlen(const char* _str)
{
	return bx::strLen(_str);
}

extern "C" size_t strnlen(const char* _str, size_t _max)
{
	return bx::strLen(_str, _max);
}

extern "C" void* strcpy(char* _dst, const char* _src)
{
	bx::strCopy(_dst, INT32_MAX, _src, INT32_MAX);
	return _dst;
}

extern "C" void* strncpy(char* _dst, const char* _src, size_t _num)
{
	bx::strCopy(_dst, INT32_MAX, _src, _num);
	return _dst;
}

extern "C" char* strcat(char* _dst, const char* _src)
{
	bx::strCat(_dst, INT32_MAX, _src, INT32_MAX);
	return _dst;
}

extern "C" const char* strchr(const char* _str, int _ch)
{
	return bx::strFind(_str, _ch).getPtr();
}

extern "C" int32_t strcmp(const char* _lhs, const char* _rhs)
{
	return bx::strCmp(_lhs, _rhs);
}

extern "C" int32_t strncmp(const char* _lhs, const char* _rhs, size_t _max)
{
	return bx::strCmp(_lhs, _rhs, _max);
}

extern "C" int32_t strcasecmp(const char* _lhs, const char* _rhs)
{
	return bx::strCmpI(_lhs, _rhs);
}

extern "C" const char* strstr(const char* _str, const char* _find)
{
	return bx::strFind(_str, _find).getPtr();
}

extern "C" void qsort(void* _base, size_t _num, size_t _size, bx::ComparisonFn _fn)
{
	BX_ASSERT(_num <= UINT32_MAX && _size <= UINT32_MAX, "");
	return bx::quickSort(_base, _num, _size, _fn);
}

extern "C" int isprint(int _ch)
{
	return bx::isPrint(_ch);
}

extern "C" int toupper(int _ch)
{
	return bx::toUpper(_ch);
}

extern "C" size_t mbstowcs(wchar_t* _dst, const char* _src, size_t _max)
{
	BX_UNUSED(_dst, _src, _max);
	return 0;
}

extern "C" char* strdup(const char* _src)
{
	uint32_t size = bx::strLen(_src)+1;
	char* dup = (char*)malloc(size);
	bx::strCopy(dup, size, _src);
	return dup;
}

extern "C" long int strtol(const char* _str, char** _end, int _base)
{
	BX_UNUSED(_str, _end, _base);
	NOT_IMPLEMENTED();
	return -1;
}

extern "C" int abs(int _value)
{
	return _value >= 0 ? _value : -_value;
}

extern "C" float fabsf(float _x)
{
	return bx::abs(_x);
}

extern "C" double fabs(double _x)
{
	return bx::abs(_x);
}

extern "C" double ldexp(double _x, int _exp)
{
	return bx::ldexp(float(_x), _exp);
}

extern "C" float expf(float _x)
{
	return bx::exp(_x);
}

extern "C" float logf(float _x)
{
	return bx::log(_x);
}

extern "C" float log10f(float _x)
{
	BX_UNUSED(_x);
	return 0.0f;
}

extern "C" float powf(float _x, float _y)
{
	return bx::pow(_x, _y);
}

extern "C" double pow(double _x, float _y)
{
	return bx::pow(_x, _y);
}

extern "C" float sinf(float _x)
{
	return bx::sin(_x);
}

extern "C" float cosf(float _x)
{
	return bx::cos(_x);
}

extern "C" float tanf(float _x)
{
	return bx::tan(_x);
}

extern "C" float atan2f(float _y, float _x)
{
	return bx::atan2(_y, _x);
}

extern "C" float sqrtf(float _x)
{
	return bx::sqrt(_x);
}

extern "C" double sqrt(double _x)
{
	return bx::sqrt(_x);
}

extern "C" float ceilf(float _x)
{
	return bx::ceil(_x);
}

extern "C" double ceil(double _x)
{
	return bx::ceil(_x);
}

extern "C" float floorf(float _x)
{
	return bx::floor(_x);
}

extern "C" double floor(double _x)
{
	return bx::floor(_x);
}

extern "C" float acosf(float _x)
{
	return bx::acos(_x);
}

extern "C" float fmodf(float _numer, float _denom)
{
	return bx::mod(_numer, _denom);
}

extern "C" int atoi(const char* _str)
{
	int32_t result = 0;
	bx::fromString(&result, _str);
	return result;
}

extern "C" double atof(const char* _str)
{
	double result = 0.0;
	bx::fromString(&result, _str);
	return result;
}

extern "C" struct DIR* opendir(const char* _dirname)
{
	BX_UNUSED(_dirname);
//	NOT_IMPLEMENTED();
	return NULL;
}

extern "C" struct dirent* readdir(struct DIR* _dirp)
{
	BX_UNUSED(_dirp);
	NOT_IMPLEMENTED();
	return NULL;
}

extern "C" int closedir(struct DIR* _dirp)
{
	BX_UNUSED(_dirp);
	NOT_IMPLEMENTED();
	return 0;
}

extern "C" int vsnprintf(char* _out, size_t _max, const char* _format, va_list _argList)
{
	return bx::vsnprintf(_out, _max, _format, _argList);
}

extern "C" int sprintf(char* _out, const char* _format, ...)
{
	va_list argList;
	va_start(argList, _format);
	int32_t len = bx::vsnprintf(_out, INT32_MAX, _format, argList);
	va_end(argList);
	return len;
}

extern "C" int snprintf(char* _out, size_t _max, const char* _format, ...)
{
	va_list argList;
	va_start(argList, _format);
	int32_t len = bx::vsnprintf(_out, _max, _format, argList);
	va_end(argList);
	return len;
}

extern "C" int printf(const char* _format, ...)
{
	va_list argList;
	va_start(argList, _format);
	bx::WriterI* writer = bx::getStdOut();
	bx::Error err;
	int32_t len = bx::write(writer, &err, _format, argList);
	va_end(argList);
	return len;
}

struct FILE
{
};

extern "C" int fprintf(FILE* _stream, const char* _format, ...)
{
	BX_UNUSED(_stream, _format);
	return -1;
}

extern "C" int vfprintf(FILE* _stream, const char* _format, va_list _argList)
{
	BX_UNUSED(_stream, _format, _argList);
	return -1;
}

extern "C" int sscanf(const char* _str, const char* _format, ...)
{
	BX_UNUSED(_str, _format);
	return -1;
}

extern "C" int fscanf(FILE* _stream, const char* _format, ...)
{
	BX_UNUSED(_stream, _format);
	return -1;
}

FILE * stdout;

extern "C" FILE* fopen(const char* _filename, const char* _mode)
{
	BX_UNUSED(_filename, _mode);
	bx::debugPrintf("fopen(\"%s\", \"%s\");\n", _filename, _mode);
//	NOT_IMPLEMENTED();
	return NULL;
}

extern "C" int fclose(FILE* _stream)
{
	BX_UNUSED(_stream);
	bx::debugPrintf("fclose(%p);\n", _stream);
//	NOT_IMPLEMENTED();
	return 0;
}

extern "C" size_t fread(void* _ptr, size_t _size, size_t _count, FILE* _stream)
{
	BX_UNUSED(_ptr, _size, _count, _stream);
	NOT_IMPLEMENTED();
	return -1;
}

extern "C" size_t fwrite(const void* _ptr, size_t _size, size_t _count, FILE* _stream)
{
	BX_UNUSED(_ptr, _size, _count, _stream);
	NOT_IMPLEMENTED();
	return -1;
}

extern "C" int fseek(FILE* _stream, long int _offset, int _origin)
{
	BX_UNUSED(_stream, _offset, _origin);
	NOT_IMPLEMENTED();
	return -1;
}

extern "C" int fseeko64(FILE* _stream, off64_t _offset, int _whence)
{
	BX_UNUSED(_stream, _offset, _whence);
	NOT_IMPLEMENTED();
	return -1;
}

extern "C" long int ftell(FILE* _stream)
{
	BX_UNUSED(_stream);
	NOT_IMPLEMENTED();
	return -1;
}

extern "C" off64_t ftello64(FILE* _stream)
{
	BX_UNUSED(_stream);
	NOT_IMPLEMENTED();
	return -1;
}

extern "C" int feof(FILE* _stream)
{
	BX_UNUSED(_stream);
	NOT_IMPLEMENTED();
	return -1;
}

extern "C" int ferror(FILE* _stream)
{
	BX_UNUSED(_stream);
	NOT_IMPLEMENTED();
	return -1;
}

extern "C" FILE* popen(const char* _command, const char* _type)
{
	BX_UNUSED(_command, _type);
	NOT_IMPLEMENTED();
	return NULL;
}

extern "C" int pclose(FILE* _stream)
{
	BX_UNUSED(_stream);
	NOT_IMPLEMENTED();
	return -1;
}

extern "C" int execvp(const char* _file, char* const _argv[])
{
	BX_UNUSED(_file, _argv);
	NOT_IMPLEMENTED();
	return -1;
}

typedef int32_t clockid_t;

inline void toTimespecNs(timespec& _ts, int64_t _nsecs)
{
	_ts.tv_sec  = _nsecs/INT64_C(1000000000);
	_ts.tv_nsec = _nsecs%INT64_C(1000000000);
}

extern "C" int clock_gettime(clockid_t _clock, struct timespec* _ts)
{
	BX_UNUSED(_clock);
	int64_t now = crt0::getHPCounter();
	toTimespecNs(*_ts, now);
	return 0;
}

extern "C" long syscall(long _num, ...)
{
	va_list argList;
	va_start(argList, _num);

	long result = -1;

	switch (_num)
	{
	case 39:
		result = crt0::processGetId();
		break;

	case 228:
		{
			clockid_t arg0 = va_arg(argList, clockid_t);
			timespec* arg1 = va_arg(argList, timespec*);
			result = clock_gettime(arg0, arg1);
		}
		break;

	default:
		bx::debugPrintf("? syscall %d\n", _num);
		break;
	}

	va_end(argList);

	return result;
}

extern "C" long sysconf(int name)
{
	BX_UNUSED(name);
	NOT_IMPLEMENTED();
	return -1;
}

extern "C" pid_t fork(void)
{
	NOT_IMPLEMENTED();
	return -1;
}

extern "C" int sched_yield(void)
{
	NOT_IMPLEMENTED();
	return -1;
}

extern "C" int prctl(int _option, unsigned long _arg2, unsigned long _arg3, unsigned long _arg4, unsigned long _arg5)
{
	BX_UNUSED(_option, _arg2, _arg3, _arg4, _arg5);
	NOT_IMPLEMENTED();
	return -1;
}

extern "C" int chdir(const char* _path)
{
	BX_UNUSED(_path);
	bx::debugPrintf("chdir(%s) not implemented!\n", _path);
	return -1;
}

extern "C" char* getcwd(char* _buf, size_t _size)
{
	BX_UNUSED(_buf, _size);
	NOT_IMPLEMENTED();
	return NULL;
}

extern "C" char* getenv(const char* _name)
{
	return const_cast<char*>(crt0::getEnv(_name) );
}

extern "C" int setenv(const char* _name, const char* _value, int _overwrite)
{
	BX_UNUSED(_name, _value, _overwrite);
	bx::debugPrintf("setenv(%s, %s, %d) not implemented!\n", _name, _value, _overwrite);
	return -1;
}

extern "C" int unsetenv(const char* _name)
{
	BX_UNUSED(_name);
	bx::debugPrintf("unsetenv(%s) not implemented!\n", _name);
	return -1;
}

#if 0
struct timeval
{
	time_t tv_sec;
	suseconds_t tv_usec;
};

struct timespec
{
	time_t tv_sec;
	long tv_nsec;
};
#endif //

extern "C" int gettimeofday(struct timeval* _tv, struct timezone* _tz)
{
	BX_UNUSED(_tz);

	timespec ts;

	if (NULL == _tv)
	{
		return 0;
	}

	clock_gettime(0 /*CLOCK_REALTIME*/, &ts);
	_tv->tv_sec = ts.tv_sec;
	_tv->tv_usec = (int)ts.tv_nsec / 1000;
	return 0;
}

typedef int64_t time_t;

extern "C" time_t time(time_t* _arg)
{
	timespec ts;
	clock_gettime(0 /*CLOCK_REALTIME*/, &ts);
	time_t result = ts.tv_sec;

	if (NULL != _arg)
	{
		*_arg = result;
	}

	return result;
}

extern "C" void* realloc(void* _ptr, size_t _size)
{
	return crt0::realloc(_ptr, _size);
}

extern "C" void* malloc(size_t _size)
{
	return crt0::realloc(NULL, _size);
}

extern "C" void free(void* _ptr)
{
	crt0::realloc(_ptr, 0);
}

#endif // BX_PLATFORM_*

extern "C" void abort(void)
{
	bx::debugPrintf("crtnone: abort called!\n");
	crt0::exit(bx::kExitFailure);
}

extern "C" void __assert_fail(const char* _assertion, const char* _file, uint32_t _line, const char* _function)
{
	BX_UNUSED(_assertion, _file, _line, _function);
	abort();
}

void* __dso_handle = (void*)&__dso_handle;

void operator delete(void*)
{
}

extern "C" void __cxa_pure_virtual(void)
{
}

extern "C" int __cxa_atexit(void (*_dtorFn)(void*), void* _arg, void* _dsoHandle)
{
	BX_UNUSED(_dtorFn, _arg, _dsoHandle);
	return 0;
}

extern "C" void __gxx_personality_v0(void)
{
}

extern "C" void _Unwind_Resume(void*)
{
}

extern "C" int __gcc_personality_v0(int _version, ...)
{
	BX_UNUSED(_version);
	return 0;
}

namespace __cxxabiv1
{
	class __class_type_info
	{
	public:
		virtual ~__class_type_info();

		const char* m_name;
	};

	__class_type_info::~__class_type_info()
	{
	}

	class __si_class_type_info : public __class_type_info
	{
	public:
		virtual ~__si_class_type_info();
	};

	__si_class_type_info::~__si_class_type_info()
	{
	}

	class __vmi_class_type_info : public __class_type_info
	{
	public:
		virtual ~__vmi_class_type_info();
	};

	__vmi_class_type_info::~__vmi_class_type_info()
	{
	}

	__extension__ typedef int __guard __attribute__( (mode(__DI__) ) );

	extern "C" int __cxa_guard_acquire(__guard* _g)
	{
		return !*(char*)(_g);
	}

	extern "C" void __cxa_guard_release(__guard* _g)
	{
		*(char*)_g = 1;
	}

	extern "C" void __cxa_guard_abort(__guard* _g)
	{
		BX_UNUSED(_g);
	}

} // namespace __cxxabiv1

#endif // BX_CRT_NONE
