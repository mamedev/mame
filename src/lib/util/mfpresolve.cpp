// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    mfpresolve.h

    Helpers for resolving member function pointers to entry points.

***************************************************************************/

#include "mfpresolve.h"

#include "osdcomm.h"

#include <cstdio>


//**************************************************************************
//  MACROS
//**************************************************************************

#if defined(MAME_DELEGATE_LOG_ADJ)
	#define LOG(...) printf(__VA_ARGS__)
#else
	#define LOG(...) do { if (false) printf(__VA_ARGS__); } while (false)
#endif



namespace util::detail {

std::pair<std::uintptr_t, std::uintptr_t> resolve_member_function_itanium(
		std::uintptr_t function,
		std::ptrdiff_t delta,
		void const *object) noexcept
{
	// apply the "this" delta to the object first - the value is shifted to the left one bit position for the ARM-like variant
	LOG("Input this=%p ptr=%p adj=%ld ", object, reinterpret_cast<void const *>(function), long(delta));
	constexpr int deltashift = (MAME_ABI_CXX_ITANIUM_MFP_TYPE == MAME_ABI_CXX_ITANIUM_MFP_ARM) ? 1 : 0;
	object = reinterpret_cast<std::uint8_t const *>(object) + (delta >> deltashift);
	LOG("Calculated this=%p ", object);

	// test the virtual member function flag - it's the low bit of either the ptr or adj field, depending on the variant
	if ((MAME_ABI_CXX_ITANIUM_MFP_TYPE == MAME_ABI_CXX_ITANIUM_MFP_ARM) ? !(delta & 1) : !(function & 1))
	{
		// conventional function pointer
		LOG("ptr=%p\n", reinterpret_cast<void const *>(function));
		return std::make_pair(function, std::uintptr_t(object));
	}
	else
	{
		// byte index into the vtable to the function
		auto const vtable_ptr = *reinterpret_cast<std::uint8_t const *const *>(object) + function - ((MAME_ABI_CXX_ITANIUM_MFP_TYPE == MAME_ABI_CXX_ITANIUM_MFP_ARM) ? 0 : 1);
		std::uintptr_t result;
		if (MAME_ABI_CXX_VTABLE_FNDESC)
			result = std::uintptr_t(vtable_ptr);
		else
			result = *reinterpret_cast<std::uintptr_t const *>(vtable_ptr);
		LOG("ptr=%p (vtable)\n", reinterpret_cast<void const *>(result));
		return std::make_pair(result, std::uintptr_t(object));
	}
}


std::tuple<std::uintptr_t, std::ptrdiff_t, bool> resolve_member_function_itanium(
		std::uintptr_t function,
		std::ptrdiff_t delta) noexcept
{
	constexpr uintptr_t funcmask = ~uintptr_t((MAME_ABI_CXX_ITANIUM_MFP_TYPE == MAME_ABI_CXX_ITANIUM_MFP_ARM) ? 0 : 1);
	constexpr int deltashift = (MAME_ABI_CXX_ITANIUM_MFP_TYPE == MAME_ABI_CXX_ITANIUM_MFP_ARM) ? 1 : 0;
	return std::make_tuple(
			function & funcmask,
			delta >> deltashift,
			(MAME_ABI_CXX_ITANIUM_MFP_TYPE == MAME_ABI_CXX_ITANIUM_MFP_ARM) ? (delta & 1) : (function & 1));
}


std::pair<std::uintptr_t, std::uintptr_t> resolve_member_function_msvc(
		void const *funcptr,
		std::size_t size,
		void const *object) noexcept
{
	mfp_msvc_unknown_equiv const *unknown;
	assert(sizeof(*unknown) >= size);
	unknown = reinterpret_cast<mfp_msvc_unknown_equiv const *>(funcptr);

	LOG("Input this=%p ", object);
	if (sizeof(mfp_msvc_single_equiv) < size)
		LOG("thisdelta=%d ", unknown->delta);
	if (sizeof(mfp_msvc_unknown_equiv) == size)
		LOG("vptrdelta=%d vindex=%d ", unknown->voffset, unknown->vindex);
	auto byteptr = reinterpret_cast<std::uint8_t const *>(object);

	// test for pointer to member function cast across virtual inheritance relationship
	if ((sizeof(mfp_msvc_unknown_equiv) == size) && unknown->vindex)
	{
		// add offset from "this" pointer to location of vptr, and add offset to virtual base from vtable
		byteptr += unknown->voffset;
		auto const vptr = *reinterpret_cast<std::uint8_t const *const *>(byteptr);
		byteptr += *reinterpret_cast<int const *>(vptr + unknown->vindex);
	}

	// add "this" pointer displacement if present in the pointer to member function
	if (sizeof(mfp_msvc_single_equiv) < size)
		byteptr += unknown->delta;
	LOG("Calculated this=%p\n", reinterpret_cast<void const *>(byteptr));

	// walk past recognisable thunks
	return std::make_pair(bypass_member_function_thunks(unknown->entrypoint, byteptr), std::uintptr_t(byteptr));
}


std::tuple<std::uintptr_t, std::ptrdiff_t, bool> resolve_member_function_msvc(
		void const *funcptr,
		std::size_t size) noexcept
{
	mfp_msvc_unknown_equiv const *unknown;
	assert(sizeof(*unknown) >= size);
	unknown = reinterpret_cast<mfp_msvc_unknown_equiv const *>(funcptr);

	// no way to represent pointer to member function cast across virtual inheritance relationship
	if ((sizeof(mfp_msvc_unknown_equiv) == size) && unknown->vindex)
		return std::make_tuple(std::uintptr_t(static_cast<void (*)()>(nullptr)), std::ptrdiff_t(0), false);

	auto const [function, is_virtual] = bypass_member_function_thunks(unknown->entrypoint);
	return std::make_tuple(
			function,
			(sizeof(mfp_msvc_single_equiv) < size) ? unknown->delta : 0,
			is_virtual);
}


std::uintptr_t bypass_member_function_thunks(
		std::uintptr_t entrypoint,
		void const *object) noexcept
{
#if defined(__x86_64__) || defined(_M_X64)
	std::uint8_t const *func = reinterpret_cast<std::uint8_t const *>(entrypoint);
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
		else if (object && (0x48 == func[0]) && (0x8b == func[1]) && (0x01 == func[2]))
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
	return std::uintptr_t(func);
#elif defined(__aarch64__) || defined(_M_ARM64)
	std::uint32_t const *func = reinterpret_cast<std::uint32_t const *>(entrypoint);
	auto const fetch = [&func] (auto offset) { return little_endianize_int32(func[offset]); };
	while (true)
	{
		if ((0x90000010 == (fetch(0) & 0x9f00001f)) && (0x91000210 == (fetch(1) & 0xffc003ff)) && (0xd61f0200 == fetch(2)))
		{
			// page-relative jump with +/-4GB reach - adrp xip0,... ; add xip0,xip0,#... ; br xip0
			LOG("Found page-relative jump at %p ", func);
			std::int64_t const page =
					(std::uint64_t(fetch(0) & 0x60000000) >> 17) |
					(std::uint64_t(fetch(0) & 0x00ffffe0) << 9) |
					((fetch(0) & 0x00800000) ? (~std::uint64_t(0) << 33) : 0);
			std::uint32_t const offset = (fetch(1) & 0x003ffc00) >> 10;
			func = reinterpret_cast<std::uint32_t const *>(((std::uintptr_t(func) + page) & (~std::uintptr_t(0) << 12)) + offset);
			LOG("redirecting to %p\n", func);
		}
		else if (object && (0xf9400010 == fetch(0)) && (0xf9400210 == (fetch(1) & 0xffc003ff)) && (0xd61f0200 == fetch(2)))
		{
			// virtual function call thunk - ldr xip0,[x0] ; ldr xip0,[x0,#...] ; br xip0
			LOG("Found virtual member function thunk at %p ", func);
			auto const vptr = *reinterpret_cast<std::uint32_t const *const *const *>(object);
			func = vptr[(fetch(1) & 0x003ffc00) >> 10];
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
	return std::uintptr_t(func);
#else
	return entrypoint;
#endif
}


std::pair<std::uintptr_t, bool> bypass_member_function_thunks(
		std::uintptr_t entrypoint) noexcept
{
#if defined(__x86_64__) || defined(_M_X64)
	std::uint8_t const *func = reinterpret_cast<std::uint8_t const *>(entrypoint);
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
				LOG("Found virtual member function thunk at %p\n", func);
				if (0x20 == func[4])        // no displacement
					return std::make_pair(std::uintptr_t(0), true);
				else if (0x60 == func[4])   // 8-bit displacement
					return std::make_pair(std::uintptr_t(*reinterpret_cast<std::int8_t const *>(func + 5)), true);
				else                        // 32-bit displacement
					return std::make_pair(std::uintptr_t(*reinterpret_cast<std::int32_t const *>(func + 5)), true);
			}
			else if ((0x48 == func[3]) && (0x8b == func[4]))
			{
				// clang virtual function call thunk - mov rax,QWORD PTR [rcx] ; mov rax,QWORD PTR [rax+...] ; jmp rax
				if  ((0x00 == func[5]) && (0x48 == func[6]) && (0xff == func[7]) && (0xe0 == func[8]))
				{
					// no displacement
					LOG("Found virtual member function thunk at %p\n", func);
					return std::make_pair(std::uintptr_t(0), true);
				}
				else if  ((0x40 == func[5]) && (0x48 == func[7]) && (0xff == func[8]) && (0xe0 == func[9]))
				{
					// 8-bit displacement
					LOG("Found virtual member function thunk at %p\n", func);
					return std::make_pair(std::uintptr_t(*reinterpret_cast<std::int8_t const *>(func + 6)), true);
				}
				else if ((0x80 == func[5]) && (0x48 == func[10]) && (0xff == func[11]) && (0xe0 == func[12]))
				{
					// 32-bit displacement
					LOG("Found virtual member function thunk at %p\n", func);
					return std::make_pair(std::uintptr_t(*reinterpret_cast<std::int32_t const *>(func + 6)), true);
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
	return std::make_pair(std::uintptr_t(func), false);
#elif defined(__aarch64__) || defined(_M_ARM64)
	std::uint32_t const *func = reinterpret_cast<std::uint32_t const *>(entrypoint);
	auto const fetch = [&func] (auto offset) { return little_endianize_int32(func[offset]); };
	while (true)
	{
		if ((0x90000010 == (fetch(0) & 0x9f00001f)) && (0x91000210 == (fetch(1) & 0xffc003ff)) && (0xd61f0200 == fetch(2)))
		{
			// page-relative jump with +/-4GB reach - adrp xip0,... ; add xip0,xip0,#... ; br xip0
			LOG("Found page-relative jump at %p ", func);
			std::int64_t const page =
					(std::uint64_t(fetch(0) & 0x60000000) >> 17) |
					(std::uint64_t(fetch(0) & 0x00ffffe0) << 9) |
					((fetch(0) & 0x00800000) ? (~std::uint64_t(0) << 33) : 0);
			std::uint32_t const offset = (fetch(1) & 0x003ffc00) >> 10;
			func = reinterpret_cast<std::uint32_t const *>(((std::uintptr_t(func) + page) & (~std::uintptr_t(0) << 12)) + offset);
			LOG("redirecting to %p\n", func);
		}
		else if ((0xf9400010 == fetch(0)) && (0xf9400210 == (fetch(1) & 0xffc003ff)) && (0xd61f0200 == fetch(2)))
		{
			// virtual function call thunk - ldr xip0,[x0] ; ldr xip0,[x0,#...] ; br xip0
			LOG("Found virtual member function thunk at %p\n", func);
			return std::make_pair(std::uintptr_t((fetch(1) & 0x003ffc00) >> (10 - 3)), true);
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
	return std::make_pair(std::uintptr_t(func), false);
#else
	return std::make_pair(entrypoint, false);
#endif
}

} // namespace util::detail
