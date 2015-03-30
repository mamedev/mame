// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  winprefix.h - Win32 prefix file, included by ALL files
//
//============================================================

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501 // Windows XP
#endif

#ifdef __GNUC__
#ifndef alloca
#define alloca  __builtin_alloca
#endif
#define min(x,y) fmin(x,y)
#define max(x,y) fmax(x,y)
#endif
