// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * ppmf.h
 *
 */

#ifndef PPMF_H_
#define PPMF_H_

#include "pconfig.h"
#include <utility>
#include <cstdint>


/*
 *
 * NL_PMF_TYPE_GNUC_PMF
 *      Use standard pointer to member function syntax C++11
 *
 *  NL_PMF_TYPE_GNUC_PMF_CONV
 *      Use gnu extension and convert the pmf to a function pointer.
 *      This is not standard compliant and needs
 *      -Wno-pmf-conversions to compile.
 *
 *  NL_PMF_TYPE_INTERNAL
 *      Use the same approach as MAME for deriving the function pointer.
 *      This is compiler-dependent as well
 *
 *  Benchmarks for ./nltool -c run -f src/mame/machine/nl_pong.cpp -t 10 -n pong_fast
 *
 *  NL_PMF_TYPE_INTERNAL:       215%	215%
 *  NL_PMF_TYPE_GNUC_PMF:       163%	196%
 *  NL_PMF_TYPE_GNUC_PMF_CONV:  215%	215%
 *  NL_PMF_TYPE_VIRTUAL:        213%	209%
 *
 *  The whole exercise was done to avoid virtual calls. In prior versions of
 *  netlist, the INTERNAL and GNUC_PMF_CONV approach provided significant improvement.
 *  Since than, "hot" was removed from functions declared as virtual.
 *  This may explain that the recent benchmarks show no difference at all.
 *
 */

#if (PPMF_TYPE == PPMF_TYPE_GNUC_PMF_CONV)
#pragma GCC diagnostic ignored "-Wpmf-conversions"
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

		template<typename FunctionType, typename MemberFunctionType, typename ObjectType>
		static FunctionType get_mfp(MemberFunctionType mftp, ObjectType *object)
		{
			mfp mfpo(mftp);
			//return mfpo.update_after_bind<FunctionType>(object);
			return reinterpret_cast<FunctionType>(
					mfpo.convert_to_generic(reinterpret_cast<generic_class *>(object)));
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

#if (PPMF_TYPE == PPMF_TYPE_PMF)
	template<typename R, typename... Targs>
	class pmfp_base
	{
	public:
		class generic_class;
		using generic_function = R (generic_class::*)(Targs...);
		pmfp_base() : m_func(nullptr) {}

		template<typename MemberFunctionType, typename O>
		void set_base(MemberFunctionType mftp, O *object)
		{
			using function_ptr = R (O::*)(Targs...);
			function_ptr t = mftp;
			m_func = reinterpret_cast<generic_function>(t);
		}
		template<typename O>
		inline R call(O *obj, Targs... args) const
		{
			using function_ptr = R (O::*)(Targs...);
			function_ptr t = reinterpret_cast<function_ptr>(m_func);
			return (obj->*t)(std::forward<Targs>(args)...);
		}
	private:
		generic_function m_func;
	};

#elif ((PPMF_TYPE == PPMF_TYPE_GNUC_PMF_CONV) || (PPMF_TYPE == PPMF_TYPE_INTERNAL))
	template<typename R, typename... Targs>
	class pmfp_base
	{
	public:
		using generic_function = void (*)();

		pmfp_base() : m_func(nullptr) {}

		template<typename MemberFunctionType, typename O>
		void set_base(MemberFunctionType mftp, O *object)
		{
	#if (PPMF_TYPE == PPMF_TYPE_INTERNAL)
			using function_ptr = MEMBER_ABI R (*)(O *obj, Targs... args);
			m_func = reinterpret_cast<generic_function>(plib::mfp::get_mfp<function_ptr>(mftp, object));
	#elif (PPMF_TYPE == PPMF_TYPE_GNUC_PMF_CONV)
			R (O::* pFunc)(Targs...) = mftp;
			m_func = reinterpret_cast<generic_function>((object->*pFunc));
	#endif
		}
		template<typename O>
		inline R call(O *obj, Targs... args) const
		{
			using function_ptr = MEMBER_ABI R (*)(O *obj, Targs... args);
			return (reinterpret_cast<function_ptr>(m_func))(obj, std::forward<Targs>(args)...);
		}
	private:
		generic_function m_func;
	};
#endif

	template<typename R, typename... Targs>
	class pmfp : public pmfp_base<R, Targs...>
	{
	public:
		class generic_class;
		pmfp() : pmfp_base<R, Targs...>(), m_obj(nullptr) {}

		template<typename MemberFunctionType, typename O>
		void set(MemberFunctionType mftp, O *object)
		{
			this->set_base(mftp, object);
			m_obj = reinterpret_cast<generic_class *>(object);
		}

		inline R operator()(Targs... args) const
		{
			return this->call(m_obj, std::forward<Targs>(args)...);
		}
	private:
		generic_class *m_obj;
	};


}

#endif /* PPMF_H_ */
