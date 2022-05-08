// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PPMF_H_
#define PPMF_H_

///
/// \file ppmf.h
///
///
/// PMF_TYPE_GNUC_PMF
///      Use standard pointer to member function syntax C++11
///
///  PMF_TYPE_GNUC_PMF_CONV
///      Use gnu extension and convert the pmf to a function pointer.
///      This is not standard compliant and needs
///      -Wno-pmf-conversions to compile.
///
///  PMF_TYPE_INTERNAL
///      Use the same approach as MAME for deriving the function pointer.
///      This is compiler-dependent as well
///
///  Benchmarks for ./nltool -c run -f src/mame/machine/nl_pong.cpp -t 10 -n pong_fast
///
///  PMF_TYPE_INTERNAL:       215%    215%
///  PMF_TYPE_GNUC_PMF:       163%    196%
///  PMF_TYPE_GNUC_PMF_CONV:  215%    215%
///
///  The whole exercise was done to avoid virtual calls. In prior versions of
///  netlist, the INTERNAL and GNUC_PMF_CONV approach provided significant improvement.
///  Since than, "hot" was removed from functions declared as virtual.
///  This may explain that the recent benchmarks show no difference at all.
///

#include "pconfig.h"
#include "ptypes.h"

#include <algorithm>
#include <cstdint> // uintptr_t
#include <utility>

//============================================================
//  Macro magic
//============================================================

//#define PPMF_TYPE 2

#define PPMF_TYPE_PMF             0
#define PPMF_TYPE_GNUC_PMF_CONV   1
#define PPMF_TYPE_INTERNAL        2

// FIXME: Remove this macro madmess latest after September, 2020
// FIXME: Do we still need to support MINGW <= 4.6?

#if defined(__clang__) && defined(__i386__) && defined(_WIN32)
	#define PHAS_PMF_INTERNAL 0
#elif defined(__GNUC__) || defined(__clang__)
	// does not work in versions over 4.7.x of 32bit MINGW
	#if defined(__MINGW32__) && !defined(__x86_64) && defined(__i386__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 7)))
		#define PHAS_PMF_INTERNAL 0
	#elif defined(__MINGW32__) && !defined(__x86_64) && defined(__i386__)
		#define PHAS_PMF_INTERNAL 1
		#define MEMBER_ABI _thiscall
	#elif defined(__arm__) || defined(__ARMEL__) || defined(__aarch64__) || defined(__MIPSEL__) || defined(__mips_isa_rev) || defined(__mips64) || defined(__EMSCRIPTEN__)
		#define PHAS_PMF_INTERNAL 2
	#else
		#define PHAS_PMF_INTERNAL 1
	#endif
#elif defined(_MSC_VER) && defined (_M_X64)
	#define PHAS_PMF_INTERNAL 3
#else
	#define PHAS_PMF_INTERNAL 0
#endif

#ifndef MEMBER_ABI
	#define MEMBER_ABI
#endif

#ifndef PPMF_TYPE
	#if (PHAS_PMF_INTERNAL > 0)
		#define PPMF_TYPE PPMF_TYPE_INTERNAL
	#else
		#define PPMF_TYPE PPMF_TYPE_PMF
	#endif
#else
	#if (PPMF_TYPE == PPMF_TYPE_INTERNAL)
		#if (PHAS_PMF_INTERNAL == 0)
			#error "Internal type not supported"
		#endif
	#else
		#undef PHAS_PMF_INTERNAL
		#define PHAS_PMF_INTERNAL 0
		#undef MEMBER_ABI
		#define MEMBER_ABI
	#endif
#endif

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
#endif

namespace plib {

	struct ppmf_internal
	{
		using ci = compile_info;
		enum { value =
			(ci::type() == ci_compiler::CLANG && !ci::m64()
				&& ci::os() == ci_os::WINDOWS)                                   ? 0 :
			(ci::mingw() && !ci::m64() && ci::version() >= 407)                  ? 0 :
			(ci::mingw() && !ci::m64())                                          ? 1 :
			((ci::type() == ci_compiler::CLANG || ci::type() == ci_compiler::GCC)
				&& (ci::arch() == ci_arch::MIPS
					|| ci::arch() == ci_arch::ARM
					|| ci::os() == ci_os::EMSCRIPTEN))                           ? 2 :
			(ci::type() == ci_compiler::CLANG || ci::type() == ci_compiler::GCC) ? 1 :
			(ci::type() == ci_compiler::MSC && ci::m64()) ?                        3 :
																				   0
		};
	};

	// FIXME: on supported platforms we should consider using GNU PMF extensions
	//        if no internal solution exists

	using ppmf_type = std::integral_constant<int, (ppmf_internal::value > 0 ? PPMF_TYPE_INTERNAL : PPMF_TYPE_PMF)>;

	// check against previous implementation
	static_assert(ppmf_internal::value == PHAS_PMF_INTERNAL, "internal mismatch");
	static_assert(ppmf_type::value == PPMF_TYPE, "type mismatch");

	///
	/// \brief Used to derive a pointer to a member function.
	///
	/// The following class was derived from the MAME delegate.h code.
	///
	template <int PMFINTERNAL>
	class mfp_raw
	{
	public:
		// construct from any member function pointer
		class generic_class;
		using generic_function = void (*)();

		template<typename MemberFunctionType>
		mfp_raw(MemberFunctionType mftp)
		: m_function(0), m_this_delta(0), m_dummy1(0), m_dummy2(0), m_size(sizeof(mfp_raw))
		{
			static_assert(sizeof(*this) >= sizeof(MemberFunctionType), "size mismatch");
			*reinterpret_cast<MemberFunctionType *>(this) = mftp; // NOLINT
			// NOLINTNEXTLINE(clang-analyzer-optin.cplusplus.UninitializedObject)
		}

	//private:
		// extract the generic function and adjust the object pointer
		void convert_to_generic(generic_function &func, generic_class *&object) const
		{
			if (PMFINTERNAL == 1)
			{
				// apply the "this" delta to the object first
				// NOLINTNEXTLINE(clang-analyzer-core.UndefinedBinaryOperatorResult,cppcoreguidelines-pro-type-reinterpret-cast)
				generic_class *o_p_delta = reinterpret_cast<generic_class *>(reinterpret_cast<std::uint8_t *>(object) + m_this_delta);

				// if the low bit of the vtable index is clear, then it is just a raw function pointer
				if ((m_function & 1) == 0)
				{
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
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
			else if (PMFINTERNAL == 2)
			{
				if ((m_this_delta & 1) == 0)
				{
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
					object = reinterpret_cast<generic_class *>(reinterpret_cast<std::uint8_t *>(object) + m_this_delta);
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
					func = reinterpret_cast<generic_function>(m_function);
				}
				else
				{
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
					object = reinterpret_cast<generic_class *>(reinterpret_cast<std::uint8_t *>(object));

					// otherwise, it is the byte index into the vtable where the actual function lives
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
					std::uint8_t *vtable_base = *reinterpret_cast<std::uint8_t **>(object);
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
					func = *reinterpret_cast<generic_function *>(vtable_base + m_function + m_this_delta - 1);
				}
			}
			else if (PMFINTERNAL == 3)
			{
				const int SINGLE_MEMFUNCPTR_SIZE = sizeof(void (generic_class::*)());

				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
				func = reinterpret_cast<generic_function>(m_function);
				if (m_size == SINGLE_MEMFUNCPTR_SIZE + sizeof(int))
					// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
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

	template<int PMFTYPE, int PMFINTERNAL, typename R, typename... Targs>
	struct mfp_helper
	{
		static_assert(PMFTYPE == 2 && PMFINTERNAL >= 1 && PMFINTERNAL <=3, "Invalid PMF type");

		using raw_type = mfp_raw<PMFINTERNAL>;
		using generic_function_storage = typename raw_type::generic_function;
		using generic_class = typename raw_type::generic_class;

		template <class C>
		using specific_member_function = R (C::*)(Targs...);

		template <class C>
		using const_specific_member_function = R (C::*)(Targs...) const;

		using generic_member_function = specific_member_function<generic_class>;

		template <class C>
		using member_abi_function      = MEMBER_ABI R (*)(C *obj, Targs... args);

		template<typename FunctionType, typename MemberFunctionType, typename ObjectType>
		static std::pair<FunctionType, ObjectType *> get(MemberFunctionType mftp, ObjectType *object)
		{
			raw_type mfpo(mftp);
			generic_function_storage rfunc(nullptr);
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			auto *robject = reinterpret_cast<generic_class *>(object);
			mfpo.convert_to_generic(rfunc, robject);
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			return std::make_pair(reinterpret_cast<FunctionType>(rfunc), reinterpret_cast<ObjectType *>(robject));
		}
		template<typename O>
		static R call(const member_abi_function<O> *func, O *obj, Targs&&... args) noexcept(true)
		{
			return (*func)(obj, std::forward<Targs>(args)...);
		}
	};

	template<typename R, typename... Targs>
	struct mfp_helper<PPMF_TYPE_PMF, 0, R, Targs...>
	{
		template <class C>
		using specific_member_function = R (C::*)(Targs...);

		template <class C>
		using const_specific_member_function = R (C::*)(Targs...) const;

		class generic_class;
		using generic_member_function = specific_member_function<generic_class>;

		template <class C>
		using member_abi_function      = specific_member_function<C>;

		using generic_function_storage = generic_member_function;

		template<typename FunctionType, typename MemberFunctionType, typename ObjectType>
		static std::pair<FunctionType, ObjectType *> get(MemberFunctionType mftp, ObjectType *object)
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			return std::make_pair(reinterpret_cast<FunctionType>(mftp), reinterpret_cast<ObjectType *>(object));
		}

		template<typename O>
		static R call(const member_abi_function<O> *func, O *obj, Targs&&... args) noexcept(true)
		{
			return (obj->*(*func))(std::forward<Targs>(args)...);
		}
	};

	template<typename R, typename... Targs>
	struct mfp_helper<PPMF_TYPE_GNUC_PMF_CONV, 0, R, Targs...>
	{
		template <class C>
		using specific_member_function = R (C::*)(Targs...);

		template <class C>
		using const_specific_member_function = R (C::*)(Targs...) const;

		class generic_class;
		using generic_member_function = specific_member_function<generic_class>;

		template <class C>
		using member_abi_function      = MEMBER_ABI R (*)(C *obj, Targs... args);

		using generic_function_storage = void (*)();

		template<typename FunctionType, typename MemberFunctionType, typename ObjectType>
		static std::pair<FunctionType, ObjectType *> get(MemberFunctionType mftp, ObjectType *object)
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			return std::make_pair(reinterpret_cast<FunctionType>(object->*mftp), reinterpret_cast<ObjectType *>(object));
		}
		template<typename O>
		static R call(const member_abi_function<O> *func, O *obj, Targs&&... args) noexcept(true)
		{
			return (*func)(obj, std::forward<Targs>(args)...);
		}
	};

	template<int PMFTYPE, int PMFINTERNAL, typename R, typename... Targs>
	class pmfp_base
	{
	public:
		using helper = mfp_helper<PMFTYPE, PMFINTERNAL, R, Targs...>;

		template <class C>
		using specific_member_function = typename helper::template specific_member_function<C>;

		template <class C>
		using const_specific_member_function = typename helper::template const_specific_member_function<C>;

		using generic_class = typename helper::generic_class;
		using generic_member_function = typename helper::generic_member_function;

		template <class C>
		using member_abi_function = typename helper::template  member_abi_function<C>;

		using generic_function_storage = typename helper::generic_function_storage;

		pmfp_base()
		: m_obj(nullptr)
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			auto *s = reinterpret_cast<std::uint8_t *>(&m_resolved);
			std::fill(s, s + sizeof(m_resolved), 0);
		}

		template<typename O>
		pmfp_base(specific_member_function<O> mftp, O *object)
		: m_obj(nullptr)
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			auto *s = reinterpret_cast<std::uint8_t *>(&m_resolved);
			std::fill(s, s + sizeof(m_resolved), 0);
			bind<specific_member_function<O>>(object, &mftp);
		}

		template<typename O>
		pmfp_base(const_specific_member_function<O> mftp, O *object)
		: m_obj(nullptr)
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			auto *s = reinterpret_cast<std::uint8_t *>(&m_resolved);
			std::fill(s, s + sizeof(m_resolved), 0);
			bind<const_specific_member_function<O>>(object, &mftp);
		}

		generic_class *object() const noexcept { return m_obj; }
		bool has_object() const noexcept { return m_obj != nullptr; }

		template<typename O>
		void set(specific_member_function<O> mftp, O *object)
		{
			bind<specific_member_function<O>>(object, &mftp);
		}

		R operator()(Targs... args) const noexcept(true)
		{
			return this->call(std::forward<Targs>(args)...);
		}

		operator bool() const noexcept { return m_resolved != nullptr; }
	private:
		template<typename SPC, typename O, typename MF>
		void bind(O * object, MF *fraw)
		{
			SPC pFunc;
			static_assert(sizeof(pFunc) >= sizeof(MF), "size error");
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			*reinterpret_cast<MF *>(&pFunc) = *fraw;

			auto r = helper::template get<member_abi_function<O>>(pFunc, object);

			static_assert(sizeof(m_resolved) >= sizeof(r.first), "size mismatch 1");
			static_assert(sizeof(m_resolved) >= sizeof(member_abi_function<O>), "size mismatch 2");

			//*reinterpret_cast<member_abi_function<O> *>(&m_resolved) = r.first;
			reinterpret_copy(r.first, m_resolved);
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			m_obj = reinterpret_cast<generic_class *>(r.second);
		}

		template<typename O>
		R call(O *obj, Targs&&... args) const noexcept(true)
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			return helper::call(reinterpret_cast<member_abi_function<O> *>(&m_resolved),
				obj, std::forward<Targs>(args)...);
		}

		R call(Targs&&... args) const noexcept(true)
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			return helper::call(reinterpret_cast<const member_abi_function<generic_class> *>(&m_resolved),
				m_obj, std::forward<Targs>(args)...);
		}

		generic_function_storage  m_resolved;
		generic_class    *m_obj;
	};

	template<typename R, typename... Targs>
	using pmfp = pmfp_base<ppmf_type::value, ppmf_internal::value, R, Targs...>;

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
	///     auto dele = a(this);
	///     dele(pstring("Hello World!"));
	///
	template<typename T>
	class late_pmfp
	{
	public:

		using return_type = T;

		template <class C>
		using specific_member_function = typename return_type::template specific_member_function<C>; // noexcept(true) --> c++-17

		using generic_member_function   = typename return_type::generic_member_function;

		class generic_class;

		using static_creator = return_type (*)(const generic_member_function *, generic_class *);

		late_pmfp() = default;

		template<typename O>
		late_pmfp(specific_member_function<O> mftp)
		: m_creator(creator<O>)
		{
			static_assert(sizeof(m_raw) >= sizeof(specific_member_function<O>), "size issue");
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			*reinterpret_cast<specific_member_function<O> *>(&m_raw) = mftp;
		}

		template<typename O>
		return_type operator()(O *object) const
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			return m_creator(&m_raw, reinterpret_cast<generic_class *>(object));
		}

	private:

		template <typename O>
		static return_type creator(const generic_member_function *raw, generic_class *obj)
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			auto p = reinterpret_cast<const specific_member_function<O> *>(raw);
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
