// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * ppmf.h
 *
 */

#ifndef PPMF_H_
#define PPMF_H_

#include "pconfig.h"

#include <cstdint>
#include <utility>

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
 *  NL_PMF_TYPE_INTERNAL:       215%    215%
 *  NL_PMF_TYPE_GNUC_PMF:       163%    196%
 *  NL_PMF_TYPE_GNUC_PMF_CONV:  215%    215%
 *  NL_PMF_TYPE_VIRTUAL:        213%    209%
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

#if defined(__GNUC__) && (__GNUC__ > 6)
#pragma GCC diagnostic ignored "-Wnoexcept-type"
#endif

namespace plib {
/*
 * The following class was derived from the MAME delegate.h code.
 * It derives a pointer to a member function.
 */

#if (PHAS_PMF_INTERNAL > 0)
	class mfp
	{
	public:
		// construct from any member function pointer
#ifdef _MSC_VER
		class __single_inheritance si_generic_class;
		class generic_class { };
#else
		class generic_class;
#endif
		using generic_function = void (*)();

		template<typename MemberFunctionType>
		mfp(MemberFunctionType mftp) // NOLINT(cppcoreguidelines-pro-type-member-init)

		: m_function(0), m_this_delta(0), m_size(sizeof(mfp))
		{
			*reinterpret_cast<MemberFunctionType *>(this) = mftp;
		}

		template<typename MemberFunctionType, typename FunctionType, typename ObjectType>
		static void get_mfp(MemberFunctionType mftp, FunctionType &func, ObjectType *&object)
		{
			mfp mfpo(mftp);
			//return mfpo.update_after_bind<FunctionType>(object);
			generic_function rfunc(nullptr);
			auto robject = reinterpret_cast<generic_class *>(object);
			mfpo.convert_to_generic(rfunc, robject);
			func = reinterpret_cast<FunctionType>(rfunc);
			object = reinterpret_cast<ObjectType *>(robject);
		}

	private:
		// extract the generic function and adjust the object pointer
		void convert_to_generic(generic_function &func, generic_class *&object) const
		{
			if (PHAS_PMF_INTERNAL == 1)
			{
				// apply the "this" delta to the object first
				// NOLINTNEXTLINE(clang-analyzer-core.UndefinedBinaryOperatorResult)
				auto o_p_delta = reinterpret_cast<generic_class *>(reinterpret_cast<std::uint8_t *>(object) + m_this_delta);

				// if the low bit of the vtable index is clear, then it is just a raw function pointer
				if (!(m_function & 1))
					func = reinterpret_cast<generic_function>(m_function);
				else
				{
					// otherwise, it is the byte index into the vtable where the actual function lives
					std::uint8_t *vtable_base = *reinterpret_cast<std::uint8_t **>(o_p_delta);
					func = *reinterpret_cast<generic_function *>(vtable_base + m_function - 1);
				}
				object = o_p_delta;
			}
			else if (PHAS_PMF_INTERNAL == 2)
			{
				if ((m_this_delta & 1) == 0) {
					object = reinterpret_cast<generic_class *>(reinterpret_cast<std::uint8_t *>(object) + m_this_delta);
					func = reinterpret_cast<generic_function>(m_function);
				}
				else
				{
					object = reinterpret_cast<generic_class *>(reinterpret_cast<std::uint8_t *>(object));

					// otherwise, it is the byte index into the vtable where the actual function lives
					std::uint8_t *vtable_base = *reinterpret_cast<std::uint8_t **>(object);
					func = *reinterpret_cast<generic_function *>(vtable_base + m_function + m_this_delta - 1);
				}
			}
			else if (PHAS_PMF_INTERNAL == 3)
			{
				const int SINGLE_MEMFUNCPTR_SIZE = sizeof(void (generic_class::*)());

				func = reinterpret_cast<generic_function>(m_function);
				if (m_size == SINGLE_MEMFUNCPTR_SIZE + sizeof(int))
					object = reinterpret_cast<generic_class *>(reinterpret_cast<std::uint8_t *>(object) + m_this_delta);
			}

		}

		// actual state
		uintptr_t               m_function;         // first item can be one of two things:
													//    if even, it's a pointer to the function
													//    if odd, it's the byte offset into the vtable
		int                     m_this_delta;       // delta to apply to the 'this' pointer

		int                     m_dummy1;           // only used for visual studio x64
		int                     m_dummy2;
		int                     m_size;
	};
#endif

#if (PPMF_TYPE == PPMF_TYPE_PMF)
	template<typename R, typename... Targs>
	class pmfp_base
	{
	public:
		class generic_class;
#if defined (__INTEL_COMPILER) && defined (_M_X64) // needed for "Intel(R) C++ Intel(R) 64 Compiler XE for applications running on Intel(R) 64, Version 14.0.2.176 Build 20140130" at least
		using generic_function = int [((sizeof(void *) + 4 * sizeof(int)) + (sizeof(int) - 1)) / sizeof(int)];
#elif defined(_MSC_VER) // all other cases - for MSVC maximum size is one pointer, plus 3 ints; all other implementations seem to be smaller
		using generic_function = int[((sizeof(void *) + 3 * sizeof(int)) + (sizeof(int) - 1)) / sizeof(int)];
#else
		using generic_function = R (generic_class::*)(Targs...);
#endif
		pmfp_base()
		{
			int *p = reinterpret_cast<int *>(&m_func);
			int *e = p + sizeof(generic_function) / sizeof(int);
			for (; p < e; p++)
				*p = 0;
		}

		template<typename MemberFunctionType, typename O>
		void set_base(MemberFunctionType mftp, O *object)
		{
			using function_ptr = R (O::*)(Targs...);
			function_ptr t = mftp;
			*reinterpret_cast<function_ptr *>(&m_func) = t;
		}
		template<typename O>
		inline R call(O *obj, Targs... args)
		{
			using function_ptr = R (O::*)(Targs...);
			function_ptr t = *reinterpret_cast<function_ptr *>(&m_func);
			return (obj->*t)(std::forward<Targs>(args)...);
		}
		bool is_set() {
#if defined(_MSC_VER) || (defined (__INTEL_COMPILER) && defined (_M_X64))
			int *p = reinterpret_cast<int *>(&m_func);
			int *e = p + sizeof(generic_function) / sizeof(int);
			for (; p < e; p++)
				if (*p != 0)
					return true;

			return false;
#else
			return m_func != nullptr;
#endif
		}
	private:
		generic_function m_func;
#if 0 && defined(_MSC_VER)
		int dummy[4];
#endif
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
			function_ptr func(nullptr);
			plib::mfp::get_mfp(mftp, func, object);
			m_func = reinterpret_cast<generic_function>(func);
	#elif (PPMF_TYPE == PPMF_TYPE_GNUC_PMF_CONV)
			R (O::* pFunc)(Targs...) = mftp;
			m_func = reinterpret_cast<generic_function>((object->*pFunc));
	#endif
		}
		template<typename O>
		R call(O *obj, Targs... args) const
		{
			using function_ptr = MEMBER_ABI R (*)(O *obj, Targs... args);
			return (reinterpret_cast<function_ptr>(m_func))(obj, std::forward<Targs>(args)...);
		}
		bool is_set() noexcept { return m_func != nullptr; }
		generic_function get_function() const noexcept { return m_func; }
	private:
		generic_function m_func;
	};
#endif

	template<typename R, typename... Targs>
	class pmfp : public pmfp_base<R, Targs...>
	{
	public:
		class generic_class;

		template <class C>
		using MemberFunctionType =  R (C::*)(Targs...);

		pmfp() : pmfp_base<R, Targs...>(), m_obj(nullptr) {}

		template<typename O>
		pmfp(MemberFunctionType<O> mftp, O *object)
		: pmfp_base<R, Targs...>()
		{
			this->set_base(mftp, object);
			m_obj = reinterpret_cast<generic_class *>(object);
		}

		template<typename O>
		void set(MemberFunctionType<O> mftp, O *object)
		{
			this->set_base(mftp, object);
			m_obj = reinterpret_cast<generic_class *>(object);
		}

		inline R operator()(Targs ... args)
		{
			return this->call(m_obj, std::forward<Targs>(args)...);
		}

		generic_class *object() const noexcept { return m_obj; }
		bool has_object() const noexcept { return m_obj != nullptr; }
	private:
		generic_class *m_obj;
	};


} // namespace plib

#endif /* PPMF_H_ */
