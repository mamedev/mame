// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    delegate.c

    Templates and classes to enable delegates for callbacks.

***************************************************************************/

#include <assert.h>

#include "osdcomm.h"
#include "delegate.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

#if (USE_DELEGATE_TYPE == DELEGATE_TYPE_COMPATIBLE)

delegate_mfp::raw_mfp_data delegate_mfp::s_null_mfp = { {0 }};

#endif



//**************************************************************************
//  INTERNAL DELEGATE HELPERS
//**************************************************************************

#if (USE_DELEGATE_TYPE == DELEGATE_TYPE_INTERNAL)

/**
 * @fn  delegate_generic_function delegate_mfp::convert_to_generic(delegate_generic_class *&object) const
 *
 * @brief   -------------------------------------------------
 *            delegate_convert_raw - given an object and an raw function, adjust the object base
 *            and return the actual final code pointer
 *          -------------------------------------------------.
 *
 * @param [in,out]  object  [in,out] If non-null, the object.
 *
 * @return  The given data converted to a generic.
 */

delegate_generic_function delegate_mfp::convert_to_generic(delegate_generic_class *&object) const
{
	// apply the "this" delta to the object first
	object = reinterpret_cast<delegate_generic_class *>(reinterpret_cast<UINT8 *>(object) + m_this_delta);

	// if the low bit of the vtable index is clear, then it is just a raw function pointer
	if (!(m_function & 1))
		return reinterpret_cast<delegate_generic_function>(m_function);

	// otherwise, it is the byte index into the vtable where the actual function lives
	UINT8 *vtable_base = *reinterpret_cast<UINT8 **>(object);
	return *reinterpret_cast<delegate_generic_function *>(vtable_base + m_function - 1);
}

#endif
