// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/// \file
/// \brief ABI feature macros
///
/// Macros that are useful for writing ABI-dependent code.
#ifndef MAME_LIB_UTIL_ABI_H
#define MAME_LIB_UTIL_ABI_H

#pragma once


/// \brief Itanium C++ ABI
///
/// Value of #MAME_ABI_CXX_TYPE when compiled with a variant of the
/// Itanium C++ ABI.
/// \sa MAME_ABI_CXX_TYPE MAME_ABI_CXX_MSVC
#define MAME_ABI_CXX_ITANIUM 0

/// \brief Microsoft Visual C++ ABI
///
/// Value of #MAME_ABI_CXX_TYPE when compiled with a variant of the
/// Microsoft Visual C++ ABI.
/// \sa MAME_ABI_CXX_TYPE MAME_ABI_CXX_ITANIUM
#define MAME_ABI_CXX_MSVC 1


/// \brief Standard Itanium C++ ABI member function pointers
///
/// Value of #MAME_ABI_CXX_ITANIUM_MFP_TYPE when compiled with a variant
/// of the Itanium C++ ABI  using the standard representation of
/// pointers to non-static member functions.
/// \sa MAME_ABI_CXX_ITANIUM_MFP_TYPE MAME_ABI_CXX_ITANIUM_MFP_ARM
#define MAME_ABI_CXX_ITANIUM_MFP_STANDARD 0

/// \brief ARM Itanium C++ ABI member function pointers
///
/// Value of #MAME_ABI_CXX_ITANIUM_MFP_TYPE when compiled with a variant
/// of the Itanium C++ ABI  using the 32-bit ARM representation of
/// pointers to non-static member functions.
/// \sa MAME_ABI_CXX_ITANIUM_MFP_TYPE MAME_ABI_CXX_ITANIUM_MFP_STANDARD
#define MAME_ABI_CXX_ITANIUM_MFP_ARM 1


/// \def MAME_ABI_FNDESC_SIZE
/// \brief Size of function descriptors
///
/// Size of function descriptors as a multiple of the size of a pointer,
/// or zero if function pointers point to the function entry point
/// directly.
#if (defined(__ppc64__) || defined(__PPC64__)) && !defined(__APPLE__) && !defined(__LITTLE_ENDIAN__)
	#define MAME_ABI_FNDESC_SIZE 3 // entry point (PC), TOC (R2), environment (R11)
#elif defined(__ia64__)
	#define MAME_ABI_FNDESC_SIZE 2 // GP, entry point
#else
	#define MAME_ABI_FNDESC_SIZE 0 // function pointers point to entry point directly
#endif


/// \def MAME_ABI_CXX_TYPE
/// \brief C++ ABI type
///
/// A constant representing the C++ ABI.
/// \sa MAME_ABI_CXX_ITANIUM MAME_ABI_CXX_MSVC
#if defined(_MSC_VER)
	#define MAME_ABI_CXX_TYPE MAME_ABI_CXX_MSVC
#else
	#define MAME_ABI_CXX_TYPE MAME_ABI_CXX_ITANIUM
#endif


/// \def MAME_ABI_CXX_MEMBER_CALL
/// \brief Member function calling convention qualifier
///
/// A qualifier for functions and function pointers that may be used to
/// specify that the calling convention for non-static member functions
/// should be used.
#if defined(__GNUC__) && defined(__MINGW32__) && !defined(__x86_64__) && defined(__i386__)
	#define MAME_ABI_CXX_MEMBER_CALL __thiscall
#else
	#define MAME_ABI_CXX_MEMBER_CALL
#endif


/// \def MAME_ABI_CXX_VTABLE_FNDESC
/// \brief Whether function descriptors are stored in virtual tables
///
/// Non-zero if function descriptors are stored in virtual tables
/// directly, or zero if function entries in virtual tables are
/// conventional function pointers.
/// \sa MAME_ABI_FNDESC_SIZE
#if defined(__ia64__)
	#define MAME_ABI_CXX_VTABLE_FNDESC 1 // function descriptors stored directly in vtable
#else
	#define MAME_ABI_CXX_VTABLE_FNDESC 0 // conventional function pointers in vtable
#endif


/// \def MAME_ABI_CXX_ITANIUM_MFP_TYPE
/// Itanium C++ member function representation
///
/// A constant representing the representation of pointers to non-static
/// member functions in use with the Itanium C++ ABI.  Only valid if
/// compiled with a variant of the Itanium C++ ABI.
/// \sa MAME_ABI_CXX_ITANIUM_MFP_STANDARD MAME_ABI_CXX_ITANIUM_MFP_ARM
///   MAME_ABI_CXX_TYPE
#if defined(__arm__) || defined(__ARMEL__) || defined(__aarch64__)
	#define MAME_ABI_CXX_ITANIUM_MFP_TYPE MAME_ABI_CXX_ITANIUM_MFP_ARM
#elif defined(__MIPSEL__) || defined(__mips_isa_rev) || defined(__mips64)
	#define MAME_ABI_CXX_ITANIUM_MFP_TYPE MAME_ABI_CXX_ITANIUM_MFP_ARM
#elif defined(__EMSCRIPTEN__)
	#define MAME_ABI_CXX_ITANIUM_MFP_TYPE MAME_ABI_CXX_ITANIUM_MFP_ARM
#else
	#define MAME_ABI_CXX_ITANIUM_MFP_TYPE MAME_ABI_CXX_ITANIUM_MFP_STANDARD
#endif

#endif // MAME_LIB_UTIL_ABI_H
