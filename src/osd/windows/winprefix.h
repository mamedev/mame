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
#endif

#ifdef __GNUC__
#ifndef alloca
#define alloca  __builtin_alloca
#endif
#define min(x,y) fmin(x,y)
#define max(x,y) fmax(x,y)
#endif

#define PATH_SEPARATOR      "\\"
