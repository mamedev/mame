// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pconfig.h
 *
 */

#ifndef PCONFIG_H_
#define PCONFIG_H_

#ifndef PSTANDALONE
	#define PSTANDALONE (0)
#endif

//============================================================
//  Compiling standalone
//============================================================

// Compiling without mame ?

#include <algorithm>
#include <cstdarg>

#if !(PSTANDALONE)
#include "osdcore.h"

#undef ATTR_COLD
#define ATTR_COLD

#else
#include <stdint.h>

/* not supported in GCC prior to 4.4.x */
/* ATTR_HOT and ATTR_COLD cause performance degration in 5.1 */
//#define ATTR_HOT
//#define ATTR_COLD
#define ATTR_HOT                __attribute__((hot))
#define ATTR_COLD              __attribute__((cold))

#define RESTRICT
#define EXPECTED(x)     (x)
#define UNEXPECTED(x)   (x)
#define ATTR_PRINTF(x,y)        __attribute__((format(printf, x, y)))
#define ATTR_UNUSED             __attribute__((__unused__))

/* 8-bit values */
typedef unsigned char                       UINT8;
typedef signed char                         INT8;

/* 16-bit values */
typedef unsigned short                      UINT16;
typedef signed short                        INT16;

/* 32-bit values */
#ifndef _WINDOWS_H
typedef unsigned int                        UINT32;
typedef signed int                          INT32;
#endif

/* 64-bit values */
#ifndef _WINDOWS_H
#ifdef _MSC_VER
typedef signed __int64                      INT64;
typedef unsigned __int64                    UINT64;
#else
typedef uint64_t    UINT64;
typedef int64_t      INT64;
#endif
#endif

/* U64 and S64 are used to wrap long integer constants. */
#if defined(__GNUC__) || defined(_MSC_VER)
#define U64(val) val##ULL
#define S64(val) val##LL
#else
#define U64(val) val
#define S64(val) val
#endif

#endif

#endif /* PCONFIG_H_ */
