// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PPMF_H_
#define PPMF_H_

///
/// \file ppmf.h
///
///
/// PMF_TYPE_PMF
///      Use standard pointer to member function syntax C++11
///
///  PMF_TYPE_GNUC_PMF_CONV
///      Use gnu extension and convert the pmf to a function pointer.
///      This is not standard compliant and needs
///      -Wno-pmf-conversions to compile.
///
///  PMF_TYPE_INTERNAL_*
///      Use the same approach as MAME for deriving the function pointer.
///      This is compiler-dependent as well
///
///  Benchmarks for ./nltool -c run  -t 10 -n pongf src/mame/machine/nl_pongf.cpp
///
///  PMF_TYPE_INTERNAL:       215%    215%    564%    580%
///  PMF_TYPE_GNUC_PMF:       163%    196%    516%    490%
///  PMF_TYPE_GNUC_PMF_CONV:  215%    215%    560%    575%
///
///

#include "pconfig.h"
#include "ptypes.h"

#include <algorithm>
#include <cstdint> // uintptr_t
#include <type_traits>
#include <utility>

//============================================================
//  Macro magic
//============================================================

//#define PPMF_FORCE_TYPE 0

#define PPMF_TYPE_PMF              0
#define PPMF_TYPE_GNUC_PMF_CONV    1
#define PPMF_TYPE_INTERNAL_ITANIUM 2
#define PPMF_TYPE_INTERNAL_ARM     3
#define PPMF_TYPE_INTERNAL_MSC     4

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

#ifndef PPMF_FORCE_TYPE
#define PPMF_FORCE_TYPE -1
#endif

namespace plib {

	struct ppmf_internal
	{
		using ci = compile_info;
		enum { value =
			(PPMF_FORCE_TYPE >= 0)                                               ? PPMF_FORCE_TYPE :
			(ci::type() == ci_compiler::CLANG && !ci::m64()
				&& ci::os() == ci_os::WINDOWS)                                   ? PPMF_TYPE_PMF :
			(ci::mingw() && !ci::m64() && ci::version() >= 407)                  ? PPMF_TYPE_PMF :
			(ci::mingw() && !ci::m64())                                          ? PPMF_TYPE_PMF : // Dropped support for mingw32 < 407 PPMF_TYPE_INTERNAL_ITANIUM :
			((ci::type() == ci_compiler::CLANG || ci::type() == ci_compiler::GCC)
				&& (ci::arch() == ci_arch::MIPS
					|| ci::arch() == ci_arch::ARM
					|| ci::os() == ci_os::EMSCRIPTEN))                           ? PPMF_TYPE_INTERNAL_ARM :
			(ci::type() == ci_compiler::MSC && ci::m64()) ?                        PPMF_TYPE_INTERNAL_MSC :
			(ci::type() == ci_compiler::CLANG || ci::type() == ci_compiler::GCC) ? PPMF_TYPE_INTERNAL_ITANIUM :
																				   PPMF_TYPE_PMF
		};
	};

	static_assert(!(compile_info::type() == ci_compiler::CLANG && (PPMF_FORCE_TYPE) ==  (PPMF_TYPE_GNUC_PMF_CONV)), "clang does not support PPMF_TYPE_GNUC_PMF_CONV");
	static_assert(!(compile_info::type() == ci_compiler::NVCC && (PPMF_FORCE_TYPE) ==  (PPMF_TYPE_GNUC_PMF_CONV)), "nvcc does not support PPMF_TYPE_GNUC_PMF_CONV");

	template<typename R, typename... Targs>
	struct mfp_traits
	{
		template<typename C> using specific_member_function = R (C::*)(Targs...);
		template<typename C> using const_specific_member_function = R (C::*)(Targs...) const;
		template<typename C> using member_static_ref = R (*)(C &, Targs...);
		template<typename C> using member_static_ptr = R (*)(C *, Targs...);
	};

	class mfp_generic_class;

	///
	/// \brief Used to derive a pointer to a member function.
	///
	/// The following class was derived from the MAME delegate.h code.
	///
	template <int PMFINTERNAL>
	class mfp_raw;

	template <>
	class mfp_raw<PPMF_TYPE_INTERNAL_ITANIUM>
	{
	public:
		// construct from any member function pointer
		using generic_function = void (*)();

		template<typename MemberFunctionType>
		mfp_raw(MemberFunctionType mftp)
		: m_function(0), m_this_delta(0)
		{
			static_assert(sizeof(*this) >= sizeof(MemberFunctionType), "size mismatch");
			*reinterpret_cast<MemberFunctionType *>(this) = mftp; // NOLINT
			// NOLINTNEXTLINE(clang-analyzer-optin.cplusplus.UninitializedObject)
		}

	//private:
		// extract the generic function and adjust the object pointer
		void convert_to_generic(generic_function &func, mfp_generic_class *&object) const
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
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
				func = *reinterpret_cast<generic_function *>(vtable_base + m_function - 1);
			}
			object = o_p_delta;
		}

		// actual state
		uintptr_t m_function;         // first item can be one of two things:
													//    if even, it's a pointer to the function
													//    if odd, it's the byte offset into the vtable
		ptrdiff_t m_this_delta;       // delta to apply to the 'this' pointer
	};

	template <>
	class mfp_raw<PPMF_TYPE_INTERNAL_ARM>
	{
	public:
		// construct from any member function pointer
		using generic_function = void (*)();

		template<typename MemberFunctionType>
		mfp_raw(MemberFunctionType mftp)
		: m_function(0), m_this_delta(0)
		{
			static_assert(sizeof(*this) >= sizeof(MemberFunctionType), "size mismatch");
			*reinterpret_cast<MemberFunctionType *>(this) = mftp; // NOLINT
		}

	//private:
		// extract the generic function and adjust the object pointer
		void convert_to_generic(generic_function &func, mfp_generic_class *&object) const
		{
			if ((m_this_delta & 1) == 0)
			{
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
				object = reinterpret_cast<mfp_generic_class *>(reinterpret_cast<std::uint8_t *>(object) + (m_this_delta >> 1));
				// NOLINTNEXTLINE(performance-no-int-to-ptr,cppcoreguidelines-pro-type-reinterpret-cast)
				func = reinterpret_cast<generic_function>(m_function);
			}
			else
			{
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
				object = reinterpret_cast<mfp_generic_class *>(reinterpret_cast<std::uint8_t *>(object));

				// otherwise, it is the byte index into the vtable where the actual function lives
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
				std::uint8_t *vtable_base = *reinterpret_cast<std::uint8_t **>(object);
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
				func = *reinterpret_cast<generic_function *>(vtable_base + m_function + m_this_delta - 1);
			}
		}

		// actual state
		uintptr_t               m_function;         // first item can be one of two things:
													//    if even, it's a pointer to the function
													//    if odd, it's the byte offset into the vtable
		ptrdiff_t               m_this_delta;       // delta to apply to the 'this' pointer
	};

	template <>
	class mfp_raw<PPMF_TYPE_INTERNAL_MSC>
	{
	public:
		// construct from any member function pointer
		using generic_function = void (*)();
		struct unknown_base_equiv { generic_function fptr; int thisdisp, vptrdisp, vtdisp; };
		struct single_base_equiv { generic_function fptr; };

		template<typename MemberFunctionType>
		mfp_raw(MemberFunctionType mftp)
		: m_function(0), m_this_delta(0), m_vptr_offs(0), m_vt_index(0), m_size(0)
		{
			static_assert(sizeof(*this) >= sizeof(MemberFunctionType), "size mismatch");
			*reinterpret_cast<MemberFunctionType *>(this) = mftp; // NOLINT
			m_size = sizeof(mftp); //NOLINT
		}

	//private:
		// extract the generic function and adjust the object pointer
		void convert_to_generic(generic_function &func, mfp_generic_class *&object) const
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			auto *byteptr = reinterpret_cast<std::uint8_t *>(object);

			// test for pointer to member function cast across virtual inheritance relationship
			if ((sizeof(unknown_base_equiv) == m_size) && m_vt_index)
			{
				// add offset from "this" pointer to location of vptr, and add offset to virtual base from vtable
				byteptr += m_vptr_offs;
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
				std::uint8_t const *const vptr = *reinterpret_cast<std::uint8_t const *const *>(byteptr);
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
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

		// actual state
		uintptr_t m_function;         // first item can be one of two things:
													//    if even, it's a pointer to the function
													//    if odd, it's the byte offset into the vtable
		int       m_this_delta;       // delta to apply to the 'this' pointer

		int       m_vptr_offs;     // only used for visual studio x64
		int       m_vt_index;
		unsigned  m_size;
	};

	template<int PMFINTERNAL, typename R, typename... Targs>
	struct mfp_helper
	{
	protected:
		static_assert(PMFINTERNAL >= PPMF_TYPE_INTERNAL_ITANIUM && PMFINTERNAL <= PPMF_TYPE_INTERNAL_MSC, "Invalid PMF type");

		using traits = mfp_traits<R, Targs...>;
		using generic_member_function = typename traits::template specific_member_function<mfp_generic_class>;
		using generic_member_abi_function = typename traits::template member_static_ptr<mfp_generic_class>;

		using raw_type = mfp_raw<PMFINTERNAL>;
		using generic_function_storage = typename raw_type::generic_function;

		mfp_helper()
		: m_obj(nullptr)
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			auto *s = reinterpret_cast<std::uint8_t *>(&m_resolved);
			std::fill(s, s + sizeof(m_resolved), 0);
		}

		template<typename O, typename F>
		void bind(O *object, F *mftp)
		{
			typename traits::template specific_member_function<O> pFunc;
			static_assert(sizeof(pFunc) >= sizeof(F), "size error");
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			//*reinterpret_cast<F *>(&pFunc) = *mftp;
			reinterpret_copy(*mftp, pFunc);
			raw_type mfpo(pFunc);
			generic_function_storage rfunc(nullptr);
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			auto *robject = reinterpret_cast<mfp_generic_class *>(object);
			mfpo.convert_to_generic(rfunc, robject);
			reinterpret_copy(rfunc, this->m_resolved);
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			m_obj = reinterpret_cast<mfp_generic_class *>(robject);
		}

		R call(Targs&&... args) const noexcept(true)
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			const auto* func = reinterpret_cast<const generic_member_abi_function *>(&m_resolved);
			return (*func)(m_obj, std::forward<Targs>(args)...);
		}
		generic_function_storage  m_resolved;
		mfp_generic_class    *m_obj;
	};

	template<typename R, typename... Targs>
	struct mfp_helper<PPMF_TYPE_PMF, R, Targs...>
	{
	protected:
		using traits = mfp_traits<R, Targs...>;
		using generic_member_function = typename traits::template specific_member_function<mfp_generic_class>;

		template <class C>
		using member_abi_function      = typename traits::template specific_member_function<C>;

		mfp_helper()
		: m_obj(nullptr)
		, m_stub(nullptr)
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			auto *s = reinterpret_cast<std::uint8_t *>(&m_resolved);
			std::fill(s, s + sizeof(m_resolved), 0);
		}

		template<typename O, typename F>
		void bind(O *object, F *mftp)
		{
			reinterpret_copy(*mftp, this->m_resolved);
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			m_obj = reinterpret_cast<mfp_generic_class *>(object);
			m_stub = &stub<O>;
		}

		R call(Targs&&... args) const noexcept(true)
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			auto* func = reinterpret_cast<const generic_member_function *>(&m_resolved);
			return (*m_stub)(func, m_obj, std::forward<Targs>(args)...);
		}

		generic_member_function  m_resolved;
		mfp_generic_class        *m_obj;
		R (*m_stub)(const generic_member_function *funci, mfp_generic_class *obji, Targs&&... args);

	private:
		template<typename O>
		static R stub(const generic_member_function *funci, mfp_generic_class *obji, Targs&&... args) noexcept(true)
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			auto *obj = reinterpret_cast<O *>(obji);
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			auto *func = reinterpret_cast<const member_abi_function<O> *>(funci);
			return (obj->*(*func))(std::forward<Targs>(args)...);
		}
	};

#if NVCCBUILD == 0
	template<typename R, typename... Targs>
	struct mfp_helper<PPMF_TYPE_GNUC_PMF_CONV, R, Targs...>
	{
	protected:
		using traits = mfp_traits<R, Targs...>;

		template <class C>
		using member_abi_function      = typename traits::template member_static_ptr<C>;

		mfp_helper()
		: m_obj(nullptr)
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			auto *s = reinterpret_cast<std::uint8_t *>(&m_resolved);
			std::fill(s, s + sizeof(m_resolved), 0);
		}

		template<typename O, typename F>
		void bind(O *object, F *mftp)
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			member_abi_function<O> t = reinterpret_cast<member_abi_function<O>>(object->*(*mftp));
			reinterpret_copy(t, this->m_resolved);
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			m_obj = reinterpret_cast<mfp_generic_class *>(object);
		}

		R call(Targs&&... args) const noexcept(true)
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			auto* func = reinterpret_cast<const member_abi_function<mfp_generic_class> *>(&m_resolved);
			return (*func)(m_obj, std::forward<Targs>(args)...);
		}

		member_abi_function<mfp_generic_class>  m_resolved;
		mfp_generic_class    *m_obj;
	};
#endif

	template <int PMFINTERNAL, typename R, typename... Targs>
	using pmfp_helper_select = std::conditional<
			std::is_void_v<R> ||
			std::is_scalar_v<R> ||
			std::is_reference_v<R> || PMFINTERNAL != PPMF_TYPE_INTERNAL_MSC,
			mfp_helper<PMFINTERNAL, R, Targs...>, mfp_helper<PPMF_TYPE_PMF, R, Targs...>>;

	template<int PMFINTERNAL, typename SIGNATURE> class pmfp_base;

	template<int PMFINTERNAL, typename R, typename... Targs>
	class pmfp_base<PMFINTERNAL, R (Targs...)> : public pmfp_helper_select<PMFINTERNAL, R, Targs...>::type
	{
	public:
		using helper = typename pmfp_helper_select<PMFINTERNAL, R, Targs...>::type;

		using traits = mfp_traits<R, Targs...>;

		pmfp_base()
		: helper()
		{
		}

		template<typename O, typename P>
		pmfp_base(typename traits::template specific_member_function<O> mftp, P *object)
		: helper()
		{
			this->bind(static_cast<O*>(object), &mftp);
		}

		template<typename O, typename P>
		pmfp_base(typename traits::template const_specific_member_function<O> mftp, P *object)
		: helper()
		{
			this->bind(static_cast<O*>(object), &mftp);
		}

		mfp_generic_class *object() const noexcept { return this->m_obj; }

		template<typename O>
		void set(typename traits::template specific_member_function<O> mftp, O *object)
		{
			this->bind(object, &mftp);
		}

		R operator()(Targs... args) const noexcept(true)
		{
			return this->call(std::forward<Targs>(args)...);
		}

		bool isnull() const noexcept { return this->m_resolved == nullptr; }
		explicit operator bool() const noexcept { return !isnull(); }
		bool has_object() const noexcept { return this->m_obj != nullptr; }
		bool operator==(const pmfp_base &rhs) const { return this->m_resolved == rhs.m_resolved; }
		bool operator!=(const pmfp_base &rhs) const { return !(*this == rhs); }

	private:
	};

	template<typename Signature>
	using pmfp = pmfp_base<ppmf_internal::value, Signature>;

	///
	/// \brief Class to support delegate late binding
	///
	/// When constructing delegates in constructors AND the referenced function
	/// is virtual, the vtable may not yet be fully constructed. In these cases
	/// the following class allows to construct the delegate later.
	///
	///     plib::late_pmfp<plib::pmfp<void, pstring>> a(&nld_7493::printer);
	///     // Store the a object somewhere
	///
	///     // After full construction ...
	///
	///     auto delegate_obj = a(this);
	///     delegate_obj(pstring("Hello World!"));
	///
	template<typename T>
	class late_pmfp
	{
	public:

		using return_type = T;
		using traits = typename return_type::traits;
		using generic_member_function = typename traits::template specific_member_function<mfp_generic_class>;
		using static_creator = return_type (*)(const generic_member_function *, mfp_generic_class *);

		template<typename O>
		late_pmfp(typename traits::template specific_member_function<O> mftp)
		: m_creator(creator<O>)
		{
			static_assert(sizeof(m_raw) >= sizeof(typename traits::template specific_member_function<O>), "size issue");
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			*reinterpret_cast<typename traits::template specific_member_function<O> *>(&m_raw) = mftp;
		}

		template<typename O>
		return_type operator()(O *object) const
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			return m_creator(&m_raw, reinterpret_cast<mfp_generic_class *>(object));
		}

	private:

		template <typename O>
		static return_type creator(const generic_member_function *raw, mfp_generic_class *obj)
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			auto p = reinterpret_cast<const typename traits::template specific_member_function<O> *>(raw);
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			auto o = reinterpret_cast<O *>(obj);
			return return_type(*p, o);
		}

		generic_member_function m_raw;
		static_creator m_creator;
	};

} // namespace plib

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif // PPMF_H_
