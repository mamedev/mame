// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PCONFIG_H_
#define PCONFIG_H_

///
/// \file pconfig.h
///

/// \brief More accurate measurements the processor supports RDTSCP.
///
#ifndef PHAS_RDTSCP
#define PHAS_RDTSCP (0)
#endif

/// \brief Use accurate timing measurements.
///
/// Only works if \ref PHAS_RDTSCP == 1
///
#ifndef PUSE_ACCURATE_STATS
#define PUSE_ACCURATE_STATS (0)
#endif

/// \brief Add support for the __float128 floating point type.
///
#ifndef PUSE_FLOAT128
#define PUSE_FLOAT128 (0)
#endif

/// \brief Compile with support for OPENMP
///
/// OpenMP adds about 10% to 20% performance for analog netlists.
///
#ifndef PUSE_OPENMP
#define PUSE_OPENMP              (1)
#endif

/// \brief Use aligned optimizations.
///
/// Set this to one if you want to use aligned storage optimizations.
///
#ifndef PUSE_ALIGNED_OPTIMIZATIONS
#if defined(__EMSCRIPTEN__)
#define PUSE_ALIGNED_OPTIMIZATIONS (0)
#else
#define PUSE_ALIGNED_OPTIMIZATIONS (1)
#endif
#endif

/// \brief Use aligned allocations.
///
/// Set this to one if you want to use aligned storage optimizations.
///
/// Defaults to \ref PUSE_ALIGNED_OPTIMIZATIONS.
///
#define PUSE_ALIGNED_ALLOCATION (PUSE_ALIGNED_OPTIMIZATIONS)

/// \brief Use aligned hints.
///
/// Some compilers support special functions to mark a pointer as being
/// aligned. Set this to one if you want to use these functions.
///
/// Defaults to \ref PUSE_ALIGNED_OPTIMIZATIONS.
///
#define PUSE_ALIGNED_HINTS      (PUSE_ALIGNED_OPTIMIZATIONS)

/// \brief Number of bytes for cache line alignment
///
#define PALIGN_CACHELINE        (64)

/// \brief Number of bytes for vector alignment
///
#define PALIGN_VECTOROPT        (32)

#define PALIGN_MIN_SIZE         (16)

#if (PUSE_ALIGNED_OPTIMIZATIONS)
#define PALIGNAS_CACHELINE()    PALIGNAS(PALIGN_CACHELINE)
#define PALIGNAS_VECTOROPT()    PALIGNAS(PALIGN_VECTOROPT)
#else
#define PALIGNAS_CACHELINE()
#define PALIGNAS_VECTOROPT()
#endif

// FIXME: Breaks mame build on windows mingw due to `-Wattribute`
//        also triggers `-Wattribute` on ARM
//        This is fixed on mingw version 10
// FIXME: no error on cross-compile - need further checks
#if defined(__GNUC__) && ((defined(_WIN32) && __GNUC__ < 10) || defined(__arm__) || defined(__ARMEL__))
#define PALIGNAS(x)
#else
#define PALIGNAS(x) alignas(x)
#endif

/// \brief nvcc build flag.
///
/// Set this to 101 if you are building with NVIDIA nvcc 10.1
///
#ifndef NVCCBUILD
#define NVCCBUILD (0)
#endif

// ============================================================
//  Check for CPP Version
//
//   C++11:     __cplusplus is 201103L.
//   C++14:     __cplusplus is 201402L.
//   c++17/c++1z__cplusplus is 201703L.
//
//   VS2015 returns 199711L here. This is the bug filed in
//   2012 which obviously never was picked up by MS:
//   https://connect.microsoft.com/VisualStudio/feedback/details/763051/a-value-of-predefined-macro-cplusplus-is-still-199711l
//
//
//============================================================

#if defined(_MSC_VER)
	// Ok
#elif __cplusplus == 201103L
	#error c++11 not supported - you need c++17
#elif __cplusplus == 201402L
	#error c++14 not supported - you need c++17
#elif __cplusplus == 201703L
	// Ok
#else
	#error "C++ version not supported"
#endif


#if (PUSE_FLOAT128)
typedef __float128 FLOAT128;
#endif

//============================================================
// Check for OpenMP
//============================================================

#if defined(OPENMP)
#if ( OPENMP >= 200805 )
#define PHAS_OPENMP (1)
#else
#define PHAS_OPENMP (0)
#endif
#elif defined(_OPENMP)
#if ( _OPENMP >= 200805 )
#define PHAS_OPENMP (1)
#else
#define PHAS_OPENMP (0)
#endif
#else
#define PHAS_OPENMP (0)
#endif


//============================================================
//  WARNINGS
//============================================================

#if (PUSE_OPENMP)
#if (!(PHAS_OPENMP))
//#error To use openmp compile and link with "-fopenmp"
#undef PUSE_OPENMP
#define PUSE_OPENMP (0)
#endif
#endif

#if (PUSE_FLOAT128)
#if defined(__has_include)
#if !__has_include(<quadmath.h>)
//#pragma message "disabling PUSE_FLOAT128 due to missing quadmath.h"
#undef PUSE_FLOAT128
#define PUSE_FLOAT128 (0)
#endif
#endif
#endif


#endif // PCONFIG_H_
