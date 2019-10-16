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
#ifndef PHAS_RDTSCP
#define PHAS_RDTSCP (0)
#endif

/*
 * Define this to use accurate timing measurements. Only works
 * if PHAS_RDTSCP == 1
 */
#ifndef PUSE_ACCURATE_STATS
#define PUSE_ACCURATE_STATS (0)
#endif

/*
 * Set this to one if you want to use 128 bit int for ptime.
 * This is about 5% slower on a kaby lake processor.
 */

#ifndef PHAS_INT128
#define PHAS_INT128 (0)
#endif

/*
 * OpenMP adds about 10% to 20% performance for analog
 * netlists like kidniki.
 */

#ifndef USE_OPENMP
#define USE_OPENMP              (0)
#endif

/*
 * Set this to one if you want to use aligned storage optimizations.
 */

#ifndef USE_ALIGNED_OPTIMIZATIONS
#define USE_ALIGNED_OPTIMIZATIONS (0)
#endif

#define USE_ALIGNED_ALLOCATION (USE_ALIGNED_OPTIMIZATIONS)
#define USE_ALIGNED_HINTS      (USE_ALIGNED_OPTIMIZATIONS)
/*
 * Standard alignment macros
 */

#define PALIGN_CACHELINE        (64)
#define PALIGN_VECTOROPT        (64)

#define PALIGNAS_CACHELINE()    PALIGNAS(PALIGN_CACHELINE)
#define PALIGNAS_VECTOROPT()    PALIGNAS(PALIGN_VECTOROPT)

/* Breaks mame build on windows due to -Wattribute
 * FIXME: no error on cross-compile - need further checks */
#if defined(_WIN32) && defined(__GNUC__)
#define PALIGNAS(x)
#else
#define PALIGNAS(x) alignas(x)
#endif

/*============================================================
 *  Check for CPP Version
 *
 *   C++11:     __cplusplus is 201103L.
 *   C++14:     __cplusplus is 201402L.
 *   c++17/c++1z__cplusplus is 201703L.
 *
 *   VS2015 returns 199711L here. This is the bug filed in
 *   2012 which obviously never was picked up by MS:
 *   https://connect.microsoft.com/VisualStudio/feedback/details/763051/a-value-of-predefined-macro-cplusplus-is-still-199711l
 *
 *
 *============================================================*/

#ifndef NVCCBUILD
#define NVCCBUILD (0)
#endif

#if NVCCBUILD
#define C14CONSTEXPR
#else
#if __cplusplus == 201103L
#define C14CONSTEXPR
#elif __cplusplus == 201402L
#define C14CONSTEXPR constexpr
#elif __cplusplus == 201703L
#define C14CONSTEXPR constexpr
#elif defined(_MSC_VER)
#define C14CONSTEXPR
#else
#error "C++ version not supported"
#endif
#endif

#if (PHAS_INT128)
typedef __uint128_t UINT128;
typedef __int128_t INT128;
#endif

//============================================================
// Check for OpenMP
//============================================================

#if defined(OPENMP)
#define HAS_OPENMP ( OPENMP >= 200805 )
#elif defined(_OPENMP)
#define HAS_OPENMP ( _OPENMP >= 200805 )
#else
#define HAS_OPENMP (0)
#endif


//============================================================
//  WARNINGS
//============================================================

#if (USE_OPENMP)
#if (!(HAS_OPENMP))
#error To use openmp compile and link with "-fopenmp"
#endif
#endif


#endif /* PCONFIG_H_ */
