// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  winprefix.h - Win32 prefix file, included by ALL files
//
//============================================================

#define _WIN32_WINNT 0x0501

#ifdef _MSC_VER
#include <assert.h>
#include <malloc.h>
#if _MSC_VER < 1800
#define alloca _alloca
#define round(x) floor((x) + 0.5)
#endif
#if _MSC_VER < 1500
#define vsnprintf _vsnprintf
#endif
#if _MSC_VER < 1900
#define snprintf _snprintf
#else
#pragma warning (disable: 4091)
#pragma warning (disable: 4267)
#pragma warning (disable: 4456 4457 4458 4459)
#pragma warning (disable: 4463)
#pragma warning (disable: 4838)
#pragma warning (disable: 5025 5026 5027)
#define _CRT_STDIO_LEGACY_WIDE_SPECIFIERS
#endif
#endif

#ifdef __GNUC__
#ifndef alloca
#define alloca  __builtin_alloca
#endif
#define min(x,y) fmin(x,y)
#define max(x,y) fmax(x,y)
#endif
#ifdef _MSC_VER
#if _MSC_VER < 1800
#define _USE_MATH_DEFINES
#include <math.h>
static __inline double fmin(double x, double y){ return (x < y) ? x : y; }
static __inline double fmax(double x, double y){ return (x > y) ? x : y; }
static __inline double log2(double x) { return log(x) * M_LOG2E; }
#endif
#endif

#define PATH_SEPARATOR      "\\"
