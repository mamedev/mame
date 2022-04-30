// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Vas Crabb
/***************************************************************************

    delegate.cpp

    Templates and classes to enable delegates for callbacks.

***************************************************************************/

#include "delegate.h"

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
	// apply the "this" delta to the object first - the value is shifted to the left one bit position for the ARM-like variant
	LOG("Input this=%p ptr=%p adj=%ld ", reinterpret_cast<void const *>(object), reinterpret_cast<void const *>(m_function), long(m_this_delta));
	object = reinterpret_cast<delegate_generic_class *>(
			reinterpret_cast<std::uint8_t *>(object) + (m_this_delta >> ((MAME_ABI_CXX_ITANIUM_MFP_TYPE == MAME_ABI_CXX_ITANIUM_MFP_ARM) ? 1 : 0)));
	LOG("Calculated this=%p ", reinterpret_cast<void const *>(object));

	// test the virtual member function flag - it's the low bit of either the ptr or adj field, depending on the variant
	if ((MAME_ABI_CXX_ITANIUM_MFP_TYPE == MAME_ABI_CXX_ITANIUM_MFP_ARM) ? !(m_this_delta & 1) : !(m_function & 1))
	{
		// conventional function pointer
		LOG("ptr=%p\n", reinterpret_cast<void const *>(m_function));
		return reinterpret_cast<delegate_generic_function>(m_function);
	}
	else
	{
		// byte index into the vtable to the function
		std::uint8_t const *const vtable_ptr = *reinterpret_cast<std::uint8_t const *const *>(object) + m_function - ((MAME_ABI_CXX_ITANIUM_MFP_TYPE == MAME_ABI_CXX_ITANIUM_MFP_ARM) ? 0 : 1);
		delegate_generic_function result;
		if (MAME_ABI_CXX_VTABLE_FNDESC)
			result = reinterpret_cast<delegate_generic_function>(uintptr_t(vtable_ptr));
		else
			result = *reinterpret_cast<delegate_generic_function const *>(vtable_ptr);
		LOG("ptr=%p (vtable)\n", reinterpret_cast<void const *>(result));
		return result;
	}
}


//-------------------------------------------------
//  delegate_mfp_msvc::adjust_this_pointer - given
//  an object pointer and member function pointer,
//  apply the displacement, and walk past
//  recognisable thunks
//-------------------------------------------------

delegate_generic_function delegate_mfp_msvc::adjust_this_pointer(delegate_generic_class *&object) const
{
	LOG("Input this=%p ", reinterpret_cast<void const *>(object));
	if (sizeof(single_base_equiv) < m_size)
		LOG("thisdelta=%d ", m_this_delta);
	if (sizeof(unknown_base_equiv) == m_size)
		LOG("vptrdelta=%d vindex=%d ", m_vptr_offs, m_vt_index);
	std::uint8_t *byteptr = reinterpret_cast<std::uint8_t *>(object);

	// test for pointer to member function cast across virtual inheritance relationship
	if ((sizeof(unknown_base_equiv) == m_size) && m_vt_index)
	{
		// add offset from "this" pointer to location of vptr, and add offset to virtual base from vtable
		byteptr += m_vptr_offs;
		std::uint8_t const *const vptr = *reinterpret_cast<std::uint8_t const *const *>(byteptr);
		byteptr += *reinterpret_cast<int const *>(vptr + m_vt_index);
	}

	// add "this" pointer displacement if present in the pointer to member function
	if (sizeof(single_base_equiv) < m_size)
		byteptr += m_this_delta;
	LOG("Calculated this=%p\n", reinterpret_cast<void const *>(byteptr));
	object = reinterpret_cast<delegate_generic_class *>(byteptr);

	// walk past recognisable thunks
#if defined(__x86_64__) || defined(_M_X64)
	std::uint8_t const *func = reinterpret_cast<std::uint8_t const *>(m_function);
	while (true)
	{
		// Assumes Windows calling convention, and doesn't consider that
		// the "this" pointer could be in RDX if RCX is a pointer to
		// space for an oversize scalar result.  Since the result area
		// is uninitialised on entry, you won't see something that looks
		// like a vtable dispatch through RCX in this case - it won't
		// behave badly, it just won't bypass virtual call thunks in the
		// rare situations where the return type is an oversize scalar.
		if (0xe9 == func[0])
		{
			// relative jump with 32-bit displacement (typically a resolved PLT entry)
			LOG("Found relative jump at %p ", func);
			func += std::ptrdiff_t(5) + *reinterpret_cast<std::int32_t const *>(func + 1);
			LOG("redirecting to %p\n", func);
			continue;
		}
		else if ((0x48 == func[0]) && (0x8b == func[1]) && (0x01 == func[2]))
		{
			if ((0xff == func[3]) && ((0x20 == func[4]) || (0x60 == func[4]) || (0xa0 == func[4])))
			{
				// MSVC virtual function call thunk - mov rax,QWORD PTR [rcx] ; jmp QWORD PTR [rax+...]
				LOG("Found virtual member function thunk at %p ", func);
				std::uint8_t const *const vptr = *reinterpret_cast<std::uint8_t const *const *>(object);
				if (0x20 == func[4])        // no displacement
					func = *reinterpret_cast<std::uint8_t const *const *>(vptr);
				else if (0x60 == func[4])   // 8-bit displacement
					func = *reinterpret_cast<std::uint8_t const *const *>(vptr + *reinterpret_cast<std::int8_t const *>(func + 5));
				else                        // 32-bit displacement
					func = *reinterpret_cast<std::uint8_t const *const *>(vptr + *reinterpret_cast<std::int32_t const *>(func + 5));
				LOG("redirecting to %p\n", func);
				continue;
			}
			else if ((0x48 == func[3]) && (0x8b == func[4]))
			{
				// clang virtual function call thunk - mov rax,QWORD PTR [rcx] ; mov rax,QWORD PTR [rax+...] ; jmp rax
				if  ((0x00 == func[5]) && (0x48 == func[6]) && (0xff == func[7]) && (0xe0 == func[8]))
				{
					// no displacement
					LOG("Found virtual member function thunk at %p ", func);
					std::uint8_t const *const vptr = *reinterpret_cast<std::uint8_t const *const *>(object);
					func = *reinterpret_cast<std::uint8_t const *const *>(vptr);
					LOG("redirecting to %p\n", func);
					continue;
				}
				else if  ((0x40 == func[5]) && (0x48 == func[7]) && (0xff == func[8]) && (0xe0 == func[9]))
				{
					// 8-bit displacement
					LOG("Found virtual member function thunk at %p ", func);
					std::uint8_t const *const vptr = *reinterpret_cast<std::uint8_t const *const *>(object);
					func = *reinterpret_cast<std::uint8_t const *const *>(vptr + *reinterpret_cast<std::int8_t const *>(func + 6));
					LOG("redirecting to %p\n", func);
					continue;
				}
				else if ((0x80 == func[5]) && (0x48 == func[10]) && (0xff == func[11]) && (0xe0 == func[12]))
				{
					// 32-bit displacement
					LOG("Found virtual member function thunk at %p ", func);
					std::uint8_t const *const vptr = *reinterpret_cast<std::uint8_t const *const *>(object);
					func = *reinterpret_cast<std::uint8_t const *const *>(vptr + *reinterpret_cast<std::int32_t const *>(func + 6));
					LOG("redirecting to %p\n", func);
					continue;
				}
			}
		}

		// clang uses unoptimised thunks if optimisation is disabled
		// Without optimisation, clang produces thunks like:
		// 50                      push    rax
		// 48 89 0c 24             mov     QWORD PTR [rsp],rcx
		// 48 8b 0c 24             mov     rcx,QWORD PTR [rsp]
		// 48 8b 01                mov     rax,QWORD PTR [rcx]
		// 48 8b 80 xx xx xx xx    mov     rax,QWORD PTR [rax+...]
		// 41 5a                   pop     r10
		// 48 ff e0                jmp     rax
		// Trying to decode these thunks likely isn't worth the effort.
		// Chasing performance in unoptimised builds isn't very useful,
		// and the format of these thunks may be fragile.

		// not something we can easily bypass
		break;
	}
	return reinterpret_cast<delegate_generic_function>(std::uintptr_t(func));
#elif defined(__aarch64__) || defined(_M_ARM64)
	std::uint32_t const *func = reinterpret_cast<std::uint32_t const *>(m_function);
	while (true)
	{
		// Assumes little Endian mode.  Instructions are always stored
		// in little Endian format on AArch64, so if big Endian mode is
		// to be supported, the values need to be swapped.
		if ((0x90000010 == (func[0] & 0x9f00001f)) && (0x91000210 == (func[1] & 0xffc003ff)) && (0xd61f0200 == func[2]))
		{
			// page-relative jump with +/-4GB reach - adrp xip0,... ; add xip0,xip0,#... ; br xip0
			LOG("Found page-relative jump at %p ", func);
			std::int64_t const page =
					(std::uint64_t(func[0] & 0x60000000) >> 17) |
					(std::uint64_t(func[0] & 0x00ffffe0) << 9) |
					((func[0] & 0x00800000) ? (~std::uint64_t(0) << 33) : 0);
			std::uint32_t const offset = (func[1] & 0x003ffc00) >> 10;
			func = reinterpret_cast<std::uint32_t const *>(((std::uintptr_t(func) + page) & (~std::uintptr_t(0) << 12)) + offset);
			LOG("redirecting to %p\n", func);
		}
		else if ((0xf9400010 == func[0]) && (0xf9400210 == (func[1] & 0xffc003ff)) && (0xd61f0200 == func[2]))
		{
			// virtual function call thunk - ldr xip0,[x0] ; ldr xip0,[x0,#...] ; br xip0
			LOG("Found virtual member function thunk at %p ", func);
			std::uint32_t const *const *const vptr = *reinterpret_cast<std::uint32_t const *const *const *>(object);
			func = vptr[(func[1] & 0x003ffc00) >> 10];
			LOG("redirecting to %p\n", func);
		}
		else
		{
			// not something we can easily bypass
			break;
		}

		// clang uses horribly sub-optimal thunks for AArch64
		// Without optimisation, clang produces thunks like:
		// d10143ff    sub  sp,sp,#80
		// f90027e7    str  x7,[sp,#72]
		// f90023e6    str  x6,[sp,#64]
		// f9001fe5    str  x5,[sp,#56]
		// f9001be4    str  x4,[sp,#48]
		// f90017e3    str  x3,[sp,#40]
		// f90013e2    str  x2,[sp,#32]
		// f9000fe1    str  x1,[sp,#24]
		// f90007e0    str  x0,[sp,#8]
		// f94007e0    ldr  x0,[sp,#8]
		// f9400009    ldr  x9,[x0]
		// f9400129    ldr  x9,[x9,#...]
		// 910143ff    add  sp,sp,#80
		// d61f0120    br   x9
		// With optimisation, clang produces thunks like:
		// d10103ff    sub  sp,sp,#64
		// a9008be1    stp  x1,x2,[sp,#8]
		// a90193e3    stp  x3,x4,[sp,#24]
		// a9029be5    stp  x5,x6,[sp,#40]
		// f9001fe7    str  x7,[sp,#56]
		// f9400009    ldr  x9,[x0]
		// f9400129    ldr  x9,[x9,#...]
		// 910103ff    add  sp,sp,#64
		// d61f0120    br   x9
		// It's more effort than it's worth to try decoding these
		// thunks.

	}
	return reinterpret_cast<delegate_generic_function>(std::uintptr_t(func));
#else
	return reinterpret_cast<delegate_generic_function>(m_function);
#endif
}

} // namespace util::detail
