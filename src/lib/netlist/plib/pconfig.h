// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pconfig.h
 *
 */

#ifndef PCONFIG_H_
#define PCONFIG_H_

/*
 * Define this for more accurate measurements if you processor supports
 * RDTSCP.
 */
#define PHAS_RDTSCP (1)

/*
 * Define this to use accurate timing measurements. Only works
 * if PHAS_RDTSCP == 1
 */
#define PUSE_ACCURATE_STATS (1)

/*
 * Set this to one if you want to use 128 bit int for ptime.
 * This is for tests only.
 */

#define PHAS_INT128 (0)

#ifndef PHAS_INT128
#define PHAS_INT128 (0)
#endif

#if (PHAS_INT128)
typedef __uint128_t UINT128;
typedef __int128_t INT128;
#endif

#if defined(__GNUC__)
#ifdef RESTRICT
#undef RESTRICT
#endif
#define RESTRICT                __restrict__
#define ATTR_UNUSED             __attribute__((__unused__))
#else
#define RESTRICT
#define ATTR_UNUSED
#endif

//============================================================
//  Standard defines
//============================================================

//============================================================
//  Pointer to Member Function
//============================================================

// This will be autodetected
//#define PPMF_TYPE 0

#define PPMF_TYPE_PMF             0
#define PPMF_TYPE_GNUC_PMF_CONV   1
#define PPMF_TYPE_INTERNAL        2

#if defined(__GNUC__)
	/* does not work in versions over 4.7.x of 32bit MINGW  */
	#if defined(__MINGW32__) && !defined(__x86_64) && defined(__i386__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 7)))
		#define PHAS_PMF_INTERNAL 0
	#elif defined(__MINGW32__) && !defined(__x86_64) && defined(__i386__)
		#define PHAS_PMF_INTERNAL 1
		#define MEMBER_ABI _thiscall
	#elif defined(__clang__) && defined(__i386__) && defined(_WIN32)
		#define PHAS_PMF_INTERNAL 0
	#elif defined(__arm__) || defined(__ARMEL__) || defined(__aarch64__) || defined(__MIPSEL__) || defined(__mips_isa_rev) || defined(__mips64) || defined(EMSCRIPTEN)
		#define PHAS_PMF_INTERNAL 2
	#else
		#define PHAS_PMF_INTERNAL 1
	#endif
#elif defined(_MSC_VER) && defined (_M_X64)
	#define PHAS_PMF_INTERNAL 3
#else
	#define PHAS_PMF_INTERNAL 0
#endif

#ifndef MEMBER_ABI
	#define MEMBER_ABI
#endif

#ifndef PPMF_TYPE
	#if (PHAS_PMF_INTERNAL > 0)
		#define PPMF_TYPE PPMF_TYPE_INTERNAL
	#else
		#define PPMF_TYPE PPMF_TYPE_PMF
	#endif
#else
	#undef PHAS_PMF_INTERNAL
	#define PHAS_PMF_INTERNAL 0
	#undef MEMBER_ABI
	#define MEMBER_ABI
#endif

#endif /* PCONFIG_H_ */
