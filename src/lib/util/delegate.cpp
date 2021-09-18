// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Vas Crabb
/***************************************************************************

    delegate.cpp

    Templates and classes to enable delegates for callbacks.

***************************************************************************/

#include "delegate.h"

#include <cstdio>


//**************************************************************************
//  MACROS
//**************************************************************************

#if defined(LOG_DELEGATES)
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...) do { if (false) printf(__VA_ARGS__); } while (false)
#endif

// some architectures use function descriptor pointers
// usually this is a global pointer value along with the branch target
// other platforms using this convention include:
// * AIX, Classic MacOS and WinNT on 32-bit POWER/PowerPC
// * pretty much anything on Itanium
#if (defined(__ppc64__) || (defined(__PPC64__))) && !defined(__APPLE__)
#define MAME_DELEGATE_VT_DESCRIPTOR 1
#endif
#ifndef MAME_DELEGATE_VT_DESCRIPTOR
#define MAME_DELEGATE_VT_DESCRIPTOR 0
#endif


namespace util::detail {

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const delegate_mfp_compatible::raw_mfp_data delegate_mfp_compatible::s_null_mfp = { { 0 } };



//**************************************************************************
//  INTERNAL DELEGATE HELPERS
//**************************************************************************

//-------------------------------------------------
//  delegate_convert_raw - given an object and an raw function, adjust the object base
//  and return the actual final code pointer
//-------------------------------------------------//

delegate_generic_function delegate_mfp_itanium::convert_to_generic(delegate_generic_class *&object) const
{
#if MAME_DELEGATE_ITANIUM_ARM
	// apply the "this" delta to the object first - remember to shift right one bit position
	object = reinterpret_cast<delegate_generic_class *>(reinterpret_cast<std::uint8_t *>(object) + (m_this_delta >> 1));
	if (!(m_this_delta & 1))
	{
		// if the low bit of the 'this' delta is clear, the pointer is a conventional function pointer
		LOG("Calculated Addr = %p\n", reinterpret_cast<void const *>(m_function));
		return reinterpret_cast<delegate_generic_function>(m_function);
	}
	else
	{
		// otherwise, it is the byte index into the vtable where the actual function lives
		std::uint8_t const *const vtable_ptr = *reinterpret_cast<std::uint8_t const *const *>(object) + m_function;
#if MAME_DELEGATE_VT_DESCRIPTOR
		delegate_generic_function const result = reinterpret_cast<delegate_generic_function>(uintptr_t(vtable_ptr));
#else // MAME_DELEGATE_VT_DESCRIPTOR
		delegate_generic_function const result = *reinterpret_cast<delegate_generic_function const *>(vtable_ptr);
#endif // MAME_DELEGATE_VT_DESCRIPTOR
		LOG("Calculated Addr = %p (VTAB)\n", reinterpret_cast<void const *>(uintptr_t(result)));
		return result;
	}
#else // MAME_DELEGATE_ITANIUM_ARM
	// apply the "this" delta to the object first
	object = reinterpret_cast<delegate_generic_class *>(reinterpret_cast<std::uint8_t *>(object) + m_this_delta);
	if (!(m_function & 1))
	{
		// if the low bit of the pointer is clear, then it is a conventional function pointer
		LOG("Calculated Addr = %p\n", reinterpret_cast<void const *>(m_function));
		return reinterpret_cast<delegate_generic_function>(m_function);
	}
	else
	{
		// otherwise, it is the byte index into the vtable where the actual function lives
		std::uint8_t const *const vtable_ptr = *reinterpret_cast<std::uint8_t const *const *>(object) + m_function - 1;
#if MAME_DELEGATE_VT_DESCRIPTOR
		delegate_generic_function const result = reinterpret_cast<delegate_generic_function>(uintptr_t(vtable_ptr));
#else // MAME_DELEGATE_VT_DESCRIPTOR
		delegate_generic_function const result = *reinterpret_cast<delegate_generic_function const *>(vtable_ptr);
#endif // MAME_DELEGATE_VT_DESCRIPTOR
		LOG("Calculated Addr = %p (VTAB)\n", reinterpret_cast<void const *>(uintptr_t(result)));
		return result;
	}
#endif // MAME_DELEGATE_ITANIUM_ARM
}

} // namespace util::detail
