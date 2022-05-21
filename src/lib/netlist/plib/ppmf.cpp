// license:BSD-3-Clause
// copyright-holders:Couriersud

#include "ppmf.h"

#if PPMF_USE_MAME_DELEGATES
#include "../../util/delegate.cpp"
#else

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

namespace plib {

	void mfp_raw<ppmf_type::INTERNAL_ITANIUM>::convert_to_generic(generic_function &func, mfp_generic_class *&object) const
	{
		// apply the "this" delta to the object first
		// NOLINTNEXTLINE(clang-analyzer-core.UndefinedBinaryOperatorResult,cppcoreguidelines-pro-type-reinterpret-cast)
		auto *o_p_delta = reinterpret_cast<mfp_generic_class *>(reinterpret_cast<std::uint8_t *>(object) + m_this_delta);

		// if the low bit of the vtable index is clear, then it is just a raw function pointer
		if ((m_function & 1) == 0)
		{
			// NOLINTNEXTLINE(performance-no-int-to-ptr,cppcoreguidelines-pro-type-reinterpret-cast)
			func = reinterpret_cast<generic_function>(m_function);
		}
		else
		{
			// otherwise, it is the byte index into the vtable where the actual function lives
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			std::uint8_t *vtable_base = *reinterpret_cast<std::uint8_t **>(o_p_delta);
			if (compile_info::abi_vtable_function_descriptors::value)
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
				func = reinterpret_cast<generic_function>(uintptr_t(vtable_base + m_function - 1));
			else
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
				func = *reinterpret_cast<generic_function *>(vtable_base + m_function - 1);
		}
		object = o_p_delta;
	}

	void mfp_raw<ppmf_type::INTERNAL_ARM>::convert_to_generic(generic_function &func, mfp_generic_class *&object) const
	{
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		object = reinterpret_cast<mfp_generic_class *>(reinterpret_cast<std::uint8_t *>(object) + (m_this_delta >> 1));
		if ((m_this_delta & 1) == 0)
		{
			// NOLINTNEXTLINE(performance-no-int-to-ptr,cppcoreguidelines-pro-type-reinterpret-cast)
			func = reinterpret_cast<generic_function>(m_function);
		}
		else
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			std::uint8_t *vtable_base = *reinterpret_cast<std::uint8_t **>(object) + m_function;
			if (compile_info::abi_vtable_function_descriptors::value)
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
				func = reinterpret_cast<generic_function>(uintptr_t(vtable_base));
			else
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
				func = *reinterpret_cast<generic_function *>(vtable_base);
		}
	}

	struct unknown_base_equiv_novtdisp { mfp_raw<ppmf_type::INTERNAL_MSC>::generic_function fptr; int thisdisp, vptrdisp; };

	void mfp_raw<ppmf_type::INTERNAL_MSC>::convert_to_generic(generic_function &func, mfp_generic_class *&object) const
	{
		//printf("%lx, %lx, %lx, %lx %lx\n", m_function, m_this_delta, m_vptr_offs, m_vt_index, m_size);

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		auto *byteptr = reinterpret_cast<std::uint8_t *>(object);

		// test for pointer to member function cast across virtual inheritance relationship
		//if ((sizeof(unknown_base_equiv) == m_size) && m_vt_index)
		if ((sizeof(unknown_base_equiv) == m_size) || (sizeof(unknown_base_equiv_novtdisp) == m_size))
		{
			// add index from "this" pointer to location of vptr. This actually is an index,
			// it needs to be multiplied by sizeof(void *)
			byteptr += (m_vptr_index * static_cast<ptrdiff_t>(sizeof(void *)));
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			std::uint8_t const *const vptr = *reinterpret_cast<std::uint8_t const *const *>(byteptr);
			// and add offset to virtual base from vtable
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			if (sizeof(unknown_base_equiv) == m_size)
				byteptr += *reinterpret_cast<int const *>(vptr + m_vt_index);
		}

		// add "this" pointer displacement if present in the pointer to member function
		if (sizeof(single_base_equiv) < m_size)
			byteptr += m_this_delta;
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		object = reinterpret_cast<mfp_generic_class *>(byteptr);

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
		auto const *funcx = reinterpret_cast<std::uint8_t const *>(m_function);
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
		func = reinterpret_cast<generic_function>(std::uintptr_t(funcx));
	}

} // namespace plib

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif
