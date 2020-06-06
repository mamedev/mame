// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef PCONFIG_H_
#define PCONFIG_H_

///
/// \file pconfig.h
///

/// \brief More accurate measurements if you processor supports RDTSCP.
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

/// \brief System supports INT128
///
/// Set this to one if you want to use 128 bit int for ptime.
/// This is about 10% slower on a skylake processor for pongf.
///
#ifndef PHAS_INT128
#define PHAS_INT128 (0)
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
#define PUSE_ALIGNED_OPTIMIZATIONS (0)
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
#define PALIGN_CACHELINE        (16)

/// \brief Number of bytes for vector alignment
///
#define PALIGN_VECTOROPT        (16)

#define PALIGNAS_CACHELINE()    PALIGNAS(PALIGN_CACHELINE)
#define PALIGNAS_VECTOROPT()    PALIGNAS(PALIGN_VECTOROPT)

// FIXME: Breaks mame build on windows due to -Wattribute
//        also triggers -Wattribute on ARM
// FIXME: no error on cross-compile - need further checks
#if defined(__GNUC__) && (defined(_WIN32) || defined(__arm__) || defined(__ARMEL__))
#define PALIGNAS(x)
#else
#define PALIGNAS(x) alignas(x)
#endif

/// \brief nvcc build flag.
///
/// Set this to 1 if you are building with NVIDIA nvcc
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


#if (NVCCBUILD > 0)
	#if NVCCBUILD == 101
		#define NVCC_CONSTEXPR constexpr
	#else
		#define NVCC_CONSTEXPR constexpr
	#endif
	#if __cplusplus != 201402L
		#error nvcc - use c++14 to compile
	#endif
#else
	#define NVCC_CONSTEXPR constexpr
	#if __cplusplus == 201103L
		#error c++11 not supported - you need c++14
	#elif __cplusplus == 201402L
		// Ok
	#elif __cplusplus == 201703L
		// Ok
	#elif defined(_MSC_VER)
		// Ok
	#else
		#error "C++ version not supported"
	#endif
#endif

#if (PHAS_INT128)
typedef __uint128_t UINT128;
typedef __int128_t INT128;
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


#endif // PCONFIG_H_
