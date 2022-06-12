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
///  Benchmarks for `./nltool -c run  -t 10 -n pong src/mame/machine/nl_pong.cpp`
///
///  PMF_TYPE_INTERNAL:       215%    215%    564%    580%
///  PMF_TYPE_GNUC_PMF:       163%    196%    516%    490%
///  PMF_TYPE_GNUC_PMF_CONV:  215%    215%    560%    575%
///
///

/// \brief Enable experimental code on Visual Studio builds and VS clang llvm builds
///
/// This enables experimental code which uses optimized builds the
/// ppmf_type::INTERNAL_MSC path also for complex (struct/union) return types.
/// This currently depends on whether the code can adequately determine on
/// x64 builds if the return type is returned through registers or passed as a
/// second argument as a pointer to the member function.
///
/// The experimental code uses a temporary storage for the return value. This a
/// copy overhead for causes for large sized return types a copy overhead.
/// It would be easier if we would be able to obtain the RDX register on entry
/// to the call. On MSVC this seems not to be possible since on x64 inline
/// assembly is not supported. Even with clang-cl inline assembly this was not
/// successful when optimized code was compiled. Therefore we have to live with
/// the limitations.
///
/// This code path is disabled by default currently.
///
#if !defined(PPMF_EXPERIMENTAL)
#define PPMF_EXPERIMENTAL 0
#endif

/// brief Enable using MAME delegates as a replacement for ppmf.
///
/// This define enables the use of MAME delegates (src/lib/util/delegate.h
/// as a replacement to ppmf. Enable this setting if you want to use the nltool
/// test suite (nltool -c tests) to produce comparisons to ppmf.
///
#if !defined(PPMF_USE_MAME_DELEGATES)
#define PPMF_USE_MAME_DELEGATES 0
#endif

#if PPMF_USE_MAME_DELEGATES

#include "../../util/delegate.h"

namespace plib {
	template<typename Signature>
	class pmfp : public delegate<Signature>
	{
	public:
		using basetype = delegate<Signature>;
		using basetype::basetype;
		using basetype::object;
		explicit operator bool() const { return !this->isnull(); }
	};
}
#else

#include "pconfig.h"
#include "ptypes.h"

#include <algorithm>
#include <cstddef> // ptrdiff_t
#include <cstdint> // uintptr_t
#include <type_traits>
#include <utility>

//============================================================
//  Macro magic
//============================================================

//#define PPMF_FORCE_TYPE 1

#ifndef PPMF_FORCE_TYPE
#define PPMF_FORCE_TYPE -1
#endif

namespace plib {

	enum class ppmf_type
	{
		PMF,
		GNUC_PMF_CONV,
		INTERNAL_ITANIUM,
		INTERNAL_ARM,
		INTERNAL_MSC
	};


	struct ppmf_internal_selector
	{
		using ci = compile_info;
		constexpr static ppmf_type value =
			(PPMF_FORCE_TYPE >= 0)                                               ? static_cast<ppmf_type>(PPMF_FORCE_TYPE) :
			(ci::type() == ci_compiler::CLANG && !ci::m64()
				&& ci::os() == ci_os::WINDOWS)                                   ? ppmf_type::PMF :
			(ci::mingw() && !ci::m64() && ci::version::full() >= typed_version<4,7>::full()) ? ppmf_type::PMF :
			(ci::mingw() && !ci::m64())                                          ? ppmf_type::PMF : // Dropped support for mingw32 < 407 ppmf_type::INTERNAL_ITANIUM :
			(ci::env() == ci_env::MSVC && ci::m64())                             ? ppmf_type::INTERNAL_MSC :
			((ci::type() == ci_compiler::CLANG || ci::type() == ci_compiler::GCC)
				&& (ci::arch() == ci_arch::MIPS
					|| ci::arch() == ci_arch::ARM
					|| ci::os() == ci_os::EMSCRIPTEN))                           ? ppmf_type::INTERNAL_ARM :
			(ci::type() == ci_compiler::CLANG || ci::type() == ci_compiler::GCC) ? ppmf_type::INTERNAL_ITANIUM :
																				   ppmf_type::PMF
		;
	};

	static_assert(!(compile_info::type() == ci_compiler::CLANG && ppmf_internal_selector::value ==  (ppmf_type::GNUC_PMF_CONV)), "clang does not support ppmf_type::GNUC_PMF_CONV");
	static_assert(!(compile_info::env() == ci_env::NVCC && ppmf_internal_selector::value ==  (ppmf_type::GNUC_PMF_CONV)), "nvcc does not support ppmf_type::GNUC_PMF_CONV");

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
	template <ppmf_type PMFINTERNAL>
	class mfp_raw;

	template <>
	class mfp_raw<ppmf_type::INTERNAL_ITANIUM>
	{
	public:
		// construct from any member function pointer
		using generic_function = void (*)();

		template<typename MemberFunctionType>
		mfp_raw(MemberFunctionType mftp);

		// extract the generic function and adjust the object pointer
		void convert_to_generic(generic_function &func, mfp_generic_class *&object) const;

		/// \brief Byte offset into the vtable
		///
		/// On x86-64, the vtable contains pointers to code, and function pointers
		/// are pointers to code. To obtain a function pointer for a virtual
		/// member function, you fetch a pointer to code from the vtable.
		///
		/// On traditional PPC64, the vtable contains pointers to function
		/// descriptors, and function pointers are pointers to function descriptors.
		/// To obtain a function pointer for a virtual member function, you
		/// fetch a pointer to a function descriptor from the vtable.
		///
		/// On IA64, the vtable contains function descriptors, and function
		/// pointers are pointers to function descriptors. To obtain a
		/// function pointer for a virtual member function, you calculate
		/// the address of the function descriptor in the vtable.
		///
		/// Simply adding the byte offset to the vtable pointer creates a
		/// function pointer on IA64 because the vtable contains function
		/// descriptors; on most other targets, the vtable contains function
		/// pointers, so you need to fetch the function pointer after
		/// calculating its address in the vtable.
		///
		uintptr_t m_function;   // first item can be one of two things:
								//    if even, it's a function pointer
								//    if odd, it's the byte offset into the vtable
		ptrdiff_t m_this_delta; // delta to apply to the 'this' pointer
	};

	template <>
	class mfp_raw<ppmf_type::INTERNAL_ARM>
	{
	public:
		// construct from any member function pointer
		using generic_function = void (*)();

		template<typename MemberFunctionType>
		mfp_raw(MemberFunctionType mftp);

		// extract the generic function and adjust the object pointer
		void convert_to_generic(generic_function &func, mfp_generic_class *&object) const;

		// actual state
		uintptr_t m_function;   // first item can be a function pointer or a byte offset into the vtable
		ptrdiff_t m_this_delta; // delta to apply to the 'this' pointer after right shifting by one bit
								//    if even, m_function is a fuction pointer
								//    if odd, m_function is the byte offset into the vtable
	};

	template <>
	class mfp_raw<ppmf_type::INTERNAL_MSC>
	{
	public:
		// construct from any member function pointer
		using generic_function = void (*)();
		struct unknown_base_equiv { generic_function fptr; int thisdisp, vptrdisp, vtdisp; };
		struct single_base_equiv { generic_function fptr; };

		template<typename MemberFunctionType>
		mfp_raw(MemberFunctionType mftp);

		// extract the generic function and adjust the object pointer
		void convert_to_generic(generic_function &func, mfp_generic_class *&object) const;

		// actual state
		uintptr_t m_function;         // pointer to the function
		int       m_this_delta;       // delta to apply to the 'this' pointer

		int       m_vptr_index;       // index into the vptr table.
		int       m_vt_index;         // offset to be applied after vptr table lookup.
		unsigned  m_size;
	};

	template <typename R>
	using pmf_is_register_return_type = std::integral_constant<bool,
		std::is_void_v<R> ||
		std::is_scalar_v<R> ||
		std::is_reference_v<R> ||
		std::is_same_v<std::remove_cv_t<R>, compile_info::int128_type> ||
		std::is_same_v<std::remove_cv_t<R>, compile_info::uint128_type> >;

	template<ppmf_type PMFINTERNAL, typename R, typename... Targs>
	struct mfp_helper
	{
	protected:
		static_assert(PMFINTERNAL >= ppmf_type::INTERNAL_ITANIUM && PMFINTERNAL <= ppmf_type::INTERNAL_MSC, "Invalid PMF type");

		using traits = mfp_traits<R, Targs...>;
		using generic_member_function = typename traits::template specific_member_function<mfp_generic_class>;
		using generic_member_abi_function = typename traits::template member_static_ptr<mfp_generic_class>;

		using raw_type = mfp_raw<PMFINTERNAL>;
		using generic_function_storage = typename raw_type::generic_function;

		mfp_helper();

		template<typename O, typename F>
		void bind(O *object, F *mftp);

		R call(Targs&&... args) const noexcept(true)
		{
#if defined(_MSC_VER) && (PPMF_EXPERIMENTAL)
			if constexpr (pmf_is_register_return_type<R>::value)
			{
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
				const auto* func = reinterpret_cast<const generic_member_abi_function*>(&m_resolved);
				return (*func)(m_obj, std::forward<Targs>(args)...);
			}
			else
			{
				using generic_member_abi_function_alt = void (*)(mfp_generic_class *,void *, Targs...);
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
				const auto* func = reinterpret_cast<const generic_member_abi_function_alt*>(&m_resolved);
				std::uint8_t temp[sizeof(typename std::conditional<std::is_void_v<R>, void *, R>::type)];
				(*func)(m_obj, &temp[0], std::forward<Targs>(args)...);
				return *reinterpret_cast<R *>(&temp);
			}
#else
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			const auto* func = reinterpret_cast<const generic_member_abi_function*>(&m_resolved);
			return (*func)(m_obj, std::forward<Targs>(args)...);
#endif
		}

		generic_function_storage  m_resolved;
		mfp_generic_class    *m_obj;
	};

	template<typename R, typename... Targs>
	struct mfp_helper<ppmf_type::PMF, R, Targs...>
	{
	protected:
		using traits = mfp_traits<R, Targs...>;
		using generic_member_function = typename traits::template specific_member_function<mfp_generic_class>;

		template <class C>
		using member_abi_function      = typename traits::template specific_member_function<C>;

		mfp_helper();

		template<typename O, typename F>
		void bind(O *object, F *mftp);

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
		static R stub(const generic_member_function* funci, mfp_generic_class* obji, Targs&&... args) noexcept(true);
	};

	template<typename R, typename... Targs>
	struct mfp_helper<ppmf_type::GNUC_PMF_CONV, R, Targs...>
	{
	protected:
		using traits = mfp_traits<R, Targs...>;

		template <class C>
		using member_abi_function      = typename traits::template member_static_ptr<C>;

		mfp_helper();

		template<typename O, typename F>
		void bind(O *object, F *mftp);

		R call(Targs&&... args) const noexcept(true)
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			auto* func = reinterpret_cast<const member_abi_function<mfp_generic_class> *>(&m_resolved);
			return (*func)(m_obj, std::forward<Targs>(args)...);
		}

		member_abi_function<mfp_generic_class>  m_resolved;
		mfp_generic_class    *m_obj;
	};

	template <ppmf_type PMFINTERNAL, typename R, typename... Targs>
	using pmfp_helper_select = std::conditional<
		pmf_is_register_return_type<R>::value
		|| PMFINTERNAL != ppmf_type::INTERNAL_MSC || (PPMF_EXPERIMENTAL),
			mfp_helper<PMFINTERNAL, R, Targs...>, mfp_helper<ppmf_type::PMF, R, Targs...>>;

	template<ppmf_type PMFINTERNAL, typename SIGNATURE> class pmfp_base;

	template<ppmf_type PMFINTERNAL, typename R, typename... Targs>
	class pmfp_base<PMFINTERNAL, R (Targs...)> : public pmfp_helper_select<PMFINTERNAL, R, Targs...>::type
	{
		static_assert((compile_info::env::value != ci_env::NVCC) || (PMFINTERNAL != ppmf_type::GNUC_PMF_CONV), "GNUC_PMF_CONV not supported by nvcc");
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
	using pmfp = pmfp_base<ppmf_internal_selector::value, Signature>;

	///
	/// \brief Class to support delegate late binding
	///
	/// When constructing delegates in constructors AND the referenced function
	/// is virtual, the vtable may not yet be fully constructed. In these cases
	/// the following class allows to construct the delegate later.
	///
	/// ```
	///     plib::late_pmfp<plib::pmfp<void, pstring>> a(&nld_7493::printer);
	///     // Store the a object somewhere
	///
	///     // After full construction ...
	///
	///     auto delegate_obj = a(this);
	///     delegate_obj(pstring("Hello World!"));
	/// ```
	template<typename T>
	class late_pmfp
	{
	public:

		using return_type = T;
		using traits = typename return_type::traits;
		using generic_member_function = typename traits::template specific_member_function<mfp_generic_class>;
		using static_creator = return_type (*)(const generic_member_function *, mfp_generic_class *);

		template<typename O>
		late_pmfp(typename traits::template specific_member_function<O> mftp);

		template<typename O>
		return_type operator()(O *object) const
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			return m_creator(&m_raw, reinterpret_cast<mfp_generic_class *>(object));
		}

	private:

		template <typename O>
		static return_type creator(const generic_member_function *raw, mfp_generic_class *obj);

		generic_member_function m_raw;
		static_creator m_creator;
	};

	template<typename MemberFunctionType>
	mfp_raw<ppmf_type::INTERNAL_ITANIUM>::mfp_raw(MemberFunctionType mftp)
	: m_function(0), m_this_delta(0)
	{
		static_assert(sizeof(*this) >= sizeof(MemberFunctionType), "size mismatch");
		*reinterpret_cast<MemberFunctionType *>(this) = mftp; // NOLINT
		// NOLINTNEXTLINE(clang-analyzer-optin.cplusplus.UninitializedObject)
	}

	template<typename MemberFunctionType>
	mfp_raw<ppmf_type::INTERNAL_ARM>::mfp_raw(MemberFunctionType mftp)
	: m_function(0), m_this_delta(0)
	{
		static_assert(sizeof(*this) >= sizeof(MemberFunctionType), "size mismatch");
		*reinterpret_cast<MemberFunctionType *>(this) = mftp; // NOLINT
	}

	template<typename MemberFunctionType>
	mfp_raw<ppmf_type::INTERNAL_MSC>::mfp_raw(MemberFunctionType mftp)
	: m_function(0), m_this_delta(0), m_vptr_index(0), m_vt_index(0), m_size(0)
	{
		static_assert(sizeof(*this) >= sizeof(MemberFunctionType), "size mismatch");
		*reinterpret_cast<MemberFunctionType *>(this) = mftp; // NOLINT
		m_size = sizeof(mftp); //NOLINT
	}

	template<ppmf_type PMFINTERNAL, typename R, typename... Targs>
	mfp_helper<PMFINTERNAL, R, Targs...>::mfp_helper()
	: m_obj(nullptr)
	{
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		auto *s = reinterpret_cast<std::uint8_t *>(&m_resolved);
		std::fill(s, s + sizeof(m_resolved), 0);
	}

	template<ppmf_type PMFINTERNAL, typename R, typename... Targs>
	template<typename O, typename F>
	void mfp_helper<PMFINTERNAL, R, Targs...>::bind(O *object, F *mftp)
	{
		typename traits::template specific_member_function<O> pFunc;
		static_assert(sizeof(pFunc) >= sizeof(F), "size error");
		//# NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		//# *reinterpret_cast<F *>(&pFunc) = *mftp;
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

	template<typename R, typename... Targs>
	mfp_helper<ppmf_type::PMF, R, Targs...>::mfp_helper()
	: m_obj(nullptr)
	, m_stub(nullptr)
	{
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		auto *s = reinterpret_cast<std::uint8_t *>(&m_resolved);
		std::fill(s, s + sizeof(m_resolved), 0);
	}

	template<typename R, typename... Targs>
	template<typename O, typename F>
	void mfp_helper<ppmf_type::PMF, R, Targs...>::bind(O *object, F *mftp)
	{
		reinterpret_copy(*mftp, this->m_resolved);
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		m_obj = reinterpret_cast<mfp_generic_class *>(object);
		m_stub = &stub<O>;
	}

	template<typename R, typename... Targs>
	template<typename O>
	R mfp_helper<ppmf_type::PMF, R, Targs...>::stub(const generic_member_function* funci, mfp_generic_class* obji, Targs&&... args) noexcept(true)
	{
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		auto* obj = reinterpret_cast<O*>(obji);
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		auto* func = reinterpret_cast<const member_abi_function<O> *>(funci);
		return (obj->*(*func))(std::forward<Targs>(args)...);
	}

	template<typename R, typename... Targs>
	mfp_helper<ppmf_type::GNUC_PMF_CONV, R, Targs...>::mfp_helper()
	: m_obj(nullptr)
	{
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		auto *s = reinterpret_cast<std::uint8_t *>(&m_resolved);
		std::fill(s, s + sizeof(m_resolved), 0);
	}

	template<typename R, typename... Targs>
	template<typename O, typename F>
	void mfp_helper<ppmf_type::GNUC_PMF_CONV, R, Targs...>::bind(O *object, F *mftp)
	{
		// nvcc still tries to compile the code below - even when shielded with a `if constexpr`
#if !defined(__NVCC__)
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		member_abi_function<O> t = reinterpret_cast<member_abi_function<O>>(object->*(*mftp));
		reinterpret_copy(t, this->m_resolved);
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		m_obj = reinterpret_cast<mfp_generic_class *>(object);
#endif
	}

	template<typename T>
	template<typename O>
	late_pmfp<T>::late_pmfp(typename traits::template specific_member_function<O> mftp)
	: m_creator(creator<O>)
	{
		static_assert(sizeof(m_raw) >= sizeof(typename traits::template specific_member_function<O>), "size issue");
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		*reinterpret_cast<typename traits::template specific_member_function<O> *>(&m_raw) = mftp;
	}

	template<typename T>
	template<typename O>
	typename late_pmfp<T>::return_type late_pmfp<T>::creator(const typename late_pmfp<T>::generic_member_function *raw, mfp_generic_class *obj)
	{
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		auto p = reinterpret_cast<const typename late_pmfp<T>::traits::template specific_member_function<O> *>(raw);
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		auto o = reinterpret_cast<O *>(obj);
		return return_type(*p, o);
	}

} // namespace plib

#endif
#endif // PPMF_H_
