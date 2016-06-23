// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pconfig.h
 *
 */

#ifndef PCONFIG_H_
#define PCONFIG_H_

#include <cstdint>
#include <thread>
#include <chrono>

/*
 * Define this for more accurate measurements if you processor supports
 * RDTSCP.
 */
#define PHAS_RDTSCP (1)

/*
 * Define this to use accurate timing measurements. Only works
 * if PHAS_RDTSCP == 1
 */
#define PUSE_ACCURATE_STATS (1)

/*
 * Set this to one if you want to use 128 bit int for ptime.
 * This is for tests only.
 */

#define PHAS_INT128 (0)

#ifndef PHAS_INT128
#define PHAS_INT128 (0)
#endif

#if (PHAS_INT128)
typedef __uint128_t UINT128;
typedef __int128_t INT128;
#endif

#if defined(__GNUC__)
#define RESTRICT                __restrict__
#define ATTR_UNUSED             __attribute__((__unused__))
#else
#define RESTRICT
#define ATTR_UNUSED
#endif

//============================================================
//  Standard defines
//============================================================

// prevent implicit copying
#define P_PREVENT_COPYING(name)               \
	private:                                  \
		name(const name &);                   \
		name &operator=(const name &);

//============================================================
//  cut down delegate implementation
//============================================================

#if defined(__GNUC__)
	/* does not work in versions over 4.7.x of 32bit MINGW  */
	#if defined(__MINGW32__) && !defined(__x86_64) && defined(__i386__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 7)))
		#define PHAS_PMF_INTERNAL 0
	#elif defined(__MINGW32__) && !defined(__x86_64) && defined(__i386__)
		#define PHAS_PMF_INTERNAL 1
		#define MEMBER_ABI _thiscall
	#elif defined(__clang__) && defined(__i386__) && defined(_WIN32)
		#define PHAS_PMF_INTERNAL 0
	#elif defined(EMSCRIPTEN)
		#define PHAS_PMF_INTERNAL 0
	#elif defined(__arm__) || defined(__ARMEL__)
		#define PHAS_PMF_INTERNAL 0
	#else
		#define PHAS_PMF_INTERNAL 1
	#endif
#else
	#define PHAS_PMF_INTERNAL 0
#endif

#ifndef MEMBER_ABI
	#define MEMBER_ABI
#endif

namespace plib {

/*
 * The following class was derived from the MAME delegate.h code.
 * It derives a pointer to a member function.
 */

#if (PHAS_PMF_INTERNAL)
	class mfp
	{
	public:
		// construct from any member function pointer
		class generic_class;
		using generic_function = void (*)();

		template<typename MemberFunctionType>
		mfp(MemberFunctionType mftp)
		: m_function(0), m_this_delta(0)
		{
			*reinterpret_cast<MemberFunctionType *>(this) = mftp;
		}

		// binding helper
		template<typename FunctionType, typename ObjectType>
		FunctionType update_after_bind(ObjectType *object)
		{
			return reinterpret_cast<FunctionType>(
					convert_to_generic(reinterpret_cast<generic_class *>(object)));
		}
		template<typename FunctionType, typename MemberFunctionType, typename ObjectType>
		static FunctionType get_mfp(MemberFunctionType mftp, ObjectType *object)
		{
			mfp mfpo(mftp);
			return mfpo.update_after_bind<FunctionType>(object);
		}

	private:
		// extract the generic function and adjust the object pointer
		generic_function convert_to_generic(generic_class * object) const
		{
			// apply the "this" delta to the object first
			generic_class * o_p_delta = reinterpret_cast<generic_class *>(reinterpret_cast<std::uint8_t *>(object) + m_this_delta);

			// if the low bit of the vtable index is clear, then it is just a raw function pointer
			if (!(m_function & 1))
				return reinterpret_cast<generic_function>(m_function);

			// otherwise, it is the byte index into the vtable where the actual function lives
			std::uint8_t *vtable_base = *reinterpret_cast<std::uint8_t **>(o_p_delta);
			return *reinterpret_cast<generic_function *>(vtable_base + m_function - 1);
		}

		// actual state
		uintptr_t               m_function;         // first item can be one of two things:
													//    if even, it's a pointer to the function
													//    if odd, it's the byte offset into the vtable
		int                     m_this_delta;       // delta to apply to the 'this' pointer
	};

#endif

}

#endif /* PCONFIG_H_ */
