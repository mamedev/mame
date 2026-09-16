// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Vas Crabb
/***************************************************************************

    delegate.cpp

    Templates and classes to enable delegates for callbacks.

***************************************************************************/

#include "delegate.h"

#include "mfpresolve.h"

#include <cstdio>
#include <sstream>


//**************************************************************************
//  MACROS
//**************************************************************************

#if defined(MAME_DELEGATE_LOG_ADJ)
	#define LOG(...) printf(__VA_ARGS__)
#else
	#define LOG(...) do { if (false) printf(__VA_ARGS__); } while (false)
#endif



//**************************************************************************
//  LATE BINDING EXCEPTION
//**************************************************************************

binding_type_exception::binding_type_exception(std::type_info const &target_type, std::type_info const &actual_type)
	: m_target_type(&target_type)
	, m_actual_type(&actual_type)
{
	std::ostringstream os;
	os << "Error performing late bind of function expecting type " << target_type.name() << " to instance of type " << actual_type.name();
	m_what = os.str();
}


char const *binding_type_exception::what() const noexcept
{
	return m_what.c_str();
}



namespace util::detail {

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const delegate_mfp_compatible::raw_mfp_data delegate_mfp_compatible::s_null_mfp = { { 0 } };



//**************************************************************************
//  INTERNAL DELEGATE HELPERS
//**************************************************************************

//-------------------------------------------------
//  delegate_mfp_itanium::convert_to_generic -
//  given an object pointer and member function
//  pointer, apply the displacement and get the
//  actual function pointer
//-------------------------------------------------

delegate_generic_function delegate_mfp_itanium::convert_to_generic(delegate_generic_class *&object) const
{
	auto const [entrypoint, adjusted] = resolve_member_function_itanium(m_function, m_this_delta, object);
	object = reinterpret_cast<delegate_generic_class *>(adjusted);
	return reinterpret_cast<delegate_generic_function>(entrypoint);
}


//-------------------------------------------------
//  delegate_mfp_msvc::adjust_this_pointer - given
//  an object pointer and member function pointer,
//  apply the displacement, and walk past
//  recognisable thunks
//-------------------------------------------------

delegate_generic_function delegate_mfp_msvc::adjust_this_pointer(delegate_generic_class *&object) const
{
	auto const [entrypoint, adjusted] = resolve_member_function_msvc(&m_function, m_size, object);
	object = reinterpret_cast<delegate_generic_class *>(adjusted);
	return reinterpret_cast<delegate_generic_function>(entrypoint);
}

} // namespace util::detail
