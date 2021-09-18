// license:BSD-3-Clause
// copyright-holders:Aaron Giles
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


namespace util::detail {

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

#if (USE_DELEGATE_TYPE == DELEGATE_TYPE_COMPATIBLE)

const delegate_mfp::raw_mfp_data delegate_mfp::s_null_mfp = { { 0 } };

#endif



//**************************************************************************
//  INTERNAL DELEGATE HELPERS
//**************************************************************************

#if (USE_DELEGATE_TYPE == DELEGATE_TYPE_INTERNAL)

//-------------------------------------------------
//  delegate_convert_raw - given an object and an raw function, adjust the object base
//  and return the actual final code pointer
//-------------------------------------------------//

delegate_generic_function delegate_mfp::convert_to_generic(delegate_generic_class *&object) const
{
#if defined(__arm__) || defined(__ARMEL__) || defined(__aarch64__) || defined(__MIPSEL__) || defined(__mips_isa_rev) || defined(__mips64) || defined(__EMSCRIPTEN__)
	if ((m_this_delta & 1) == 0)
	{
		LOG("Calculated Addr = %p\n", reinterpret_cast<void const *>(uintptr_t(m_function)));
		object = reinterpret_cast<delegate_generic_class *>(reinterpret_cast<std::uint8_t *>(object) + m_this_delta);
		return reinterpret_cast<delegate_generic_function>(m_function);
	}
	object = reinterpret_cast<delegate_generic_class *>(reinterpret_cast<std::uint8_t *>(object));

	// otherwise, it is the byte index into the vtable where the actual function lives
	std::uint8_t *vtable_base = *reinterpret_cast<std::uint8_t **>(object);
	LOG("Calculated Addr = %p (VTAB)\n", reinterpret_cast<void const *>(uintptr_t(*reinterpret_cast<delegate_generic_function *>(vtable_base + m_function + m_this_delta - 1))));
	return *reinterpret_cast<delegate_generic_function *>(vtable_base + m_function + m_this_delta - 1);
#else
	// apply the "this" delta to the object first
	object = reinterpret_cast<delegate_generic_class *>(reinterpret_cast<std::uint8_t *>(object) + m_this_delta);

	// if the low bit of the vtable index is clear, then it is just a raw function pointer
	if (!(m_function & 1))
	{
		LOG("Calculated Addr = %p\n", reinterpret_cast<void const *>(uintptr_t(m_function)));
		return reinterpret_cast<delegate_generic_function>(m_function);
	}

	// otherwise, it is the byte index into the vtable where the actual function lives
	std::uint8_t *vtable_base = *reinterpret_cast<std::uint8_t **>(object);
	LOG("Calculated Addr = %p (VTAB)\n", reinterpret_cast<void *>(uintptr_t(*reinterpret_cast<delegate_generic_function *>(vtable_base + m_function - 1))));
	return *reinterpret_cast<delegate_generic_function *>(vtable_base + m_function - 1);
#endif
}

#endif

} // namespace util::detail
