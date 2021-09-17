// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Couriersud,Miodrag Milanovic
/***************************************************************************

    delegate.h

    Templates and classes to enable delegates for callbacks.

****************************************************************************

    There are many implementations of delegate-like functionality for
    C++ code, but none of them is a perfect drop-in fit for use in MAME.
    In order to be useful in MAME, we need the following properties:

        * No significant overhead; we want to use these for memory
          accessors, and memory accessor overhead is already the dominant
          performance aspect for most drivers.

        * Existing static functions need to be bound with an additional
          pointer parameter as the first argument. All existing
          implementations that allow static function binding assume the
          same signature as the member functions.

        * We must be able to bind the function separately from the
          object. This is to allow configurations to bind functions
          before the objects are created.

    Thus, the implementations below are based on existing works but are
    really a new implementation that is specific to MAME.

    --------------------------------------------------------------------

    The "compatible" version of delegates is based on an implementation
    from Sergey Ryazanov, found here:

        http://www.codeproject.com/KB/cpp/ImpossiblyFastCppDelegate.aspx

    These delegates essentially generate a templated static stub function
    for each target function. The static function takes the first
    parameter, uses it as the object pointer, and calls through the
    member function. For static functions, the stub is compatible with
    the signature of a static function, so we just set the stub directly.

    Pros:
        * should work with any modern compiler
        * static bindings are just as fast as direct calls

    Cons:
        * lots of little stub functions generated
        * double-hops on member function calls means more overhead
        * calling through stub functions repackages parameters

    --------------------------------------------------------------------

    The "internal" version of delegates makes use of the internal
    structure of member function pointers in order to convert them at
    binding time into simple static function pointers. This only works
    on platforms where object->func(p1, p2) is equivalent in calling
    convention to func(object, p1, p2).

    Most of the information on how this works comes from Don Clugston
    in this article:

        http://www.codeproject.com/KB/cpp/FastDelegate.aspx

    Pros:
        * as fast as a standard function call in static and member cases
        * no stub functions or double-hops needed

    Cons:
        * requires internal knowledge of the member function pointer
        * only works for GCC (for now; MSVC info is also readily available)

***************************************************************************/
#ifndef MAME_UTIL_DELEGATE_H
#define MAME_UTIL_DELEGATE_H

#pragma once

#include <any>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <exception>
#include <functional>
#include <type_traits>
#include <typeinfo>
#include <utility>


//**************************************************************************
//  MACROS
//**************************************************************************

// types of delegates supported
#define DELEGATE_TYPE_COMPATIBLE 0
#define DELEGATE_TYPE_INTERNAL 1
#define DELEGATE_TYPE_MSVC 2

// select which one we will be using
#if defined(FORCE_COMPATIBLE)
	#define USE_DELEGATE_TYPE DELEGATE_TYPE_COMPATIBLE
#elif defined(__GNUC__)
	/* 32bit MINGW  asks for different convention */
	#if defined(__MINGW32__) && !defined(__x86_64) && defined(__i386__)
		#define USE_DELEGATE_TYPE DELEGATE_TYPE_INTERNAL
		#define MEMBER_ABI __thiscall
		#define HAS_DIFFERENT_ABI 1
	#elif defined(__clang__) && defined(__i386__) && defined(_WIN32)
		#define USE_DELEGATE_TYPE DELEGATE_TYPE_COMPATIBLE
	#else
		#define USE_DELEGATE_TYPE DELEGATE_TYPE_INTERNAL
		#define MEMBER_ABI
		#define HAS_DIFFERENT_ABI 0
	#endif
#elif defined(_MSC_VER) && defined(_M_X64)
	#define MEMBER_ABI
	#define HAS_DIFFERENT_ABI 0
	#define USE_DELEGATE_TYPE DELEGATE_TYPE_MSVC
#else
	#define USE_DELEGATE_TYPE DELEGATE_TYPE_COMPATIBLE
#endif

#if (USE_DELEGATE_TYPE == DELEGATE_TYPE_COMPATIBLE)
	#define MEMBER_ABI
	#define HAS_DIFFERENT_ABI 0
#endif


namespace util::detail {

//**************************************************************************
//  HELPER CLASSES
//**************************************************************************

// generic function type
using delegate_generic_function = void(*)();


// ======================> generic_class

// define a dummy generic class that is just straight single-inheritance
#ifdef _MSC_VER
class __single_inheritance generic_class;
class delegate_generic_class { };
#else
class delegate_generic_class;
#endif


// ======================> delegate_traits

// delegate_traits is a meta-template that is used to provide a static function pointer
// and member function pointer of the appropriate type and number of parameters

template <typename ClassType, typename ReturnType, typename... Params>
struct delegate_traits
{
	using static_func_type = ReturnType (*)(ClassType *, Params...);
	using static_ref_func_type = ReturnType (*)(ClassType &, Params...);
	using member_func_type = ReturnType (ClassType::*)(Params...);
	using const_member_func_type = ReturnType (ClassType::*)(Params...) const;
};



//**************************************************************************
//  DELEGATE MEMBER FUNCTION POINTER WRAPPERS
//**************************************************************************

#if (USE_DELEGATE_TYPE == DELEGATE_TYPE_COMPATIBLE)

// ======================> delegate_mfp

// delegate_mfp is a class that wraps a generic member function pointer
// in a static buffer, and can effectively recast itself back for later use;
// it hides some of the gross details involved in copying arbitrary member
// function pointers around
class delegate_mfp
{
public:
	// default constructor
	delegate_mfp()
		: m_rawdata(s_null_mfp)
		, m_realobject(nullptr)
		, m_stubfunction(nullptr)
	{ }

	// copy constructor
	delegate_mfp(const delegate_mfp &src) = default;

	// construct from any member function pointer
	template <typename MemberFunctionType, class MemberFunctionClass, typename ReturnType, typename StaticFunctionType>
	delegate_mfp(MemberFunctionType mfp, MemberFunctionClass *, ReturnType *, StaticFunctionType)
		: m_rawdata(s_null_mfp)
		, m_realobject(nullptr)
		, m_stubfunction(make_generic<StaticFunctionType>(&delegate_mfp::method_stub<MemberFunctionClass, ReturnType>))
	{
		static_assert(sizeof(mfp) <= sizeof(m_rawdata), "Unsupported member function pointer size");
		*reinterpret_cast<MemberFunctionType *>(&m_rawdata) = mfp;
	}

	// comparison helpers
	bool operator==(const delegate_mfp &rhs) const { return (m_rawdata == rhs.m_rawdata); }
	bool isnull() const { return (m_rawdata == s_null_mfp); }

	// getters
	delegate_generic_class *real_object(delegate_generic_class *original) const { return m_realobject; }

	// binding helper
	template <typename FunctionType>
	void update_after_bind(FunctionType &funcptr, delegate_generic_class *&object)
	{
		m_realobject = object;
		object = reinterpret_cast<delegate_generic_class *>(this);
		funcptr = reinterpret_cast<FunctionType>(m_stubfunction);
	}

private:
	// helper stubs for calling encased member function pointers
	template <class FunctionClass, typename ReturnType, typename... Params>
	static ReturnType method_stub(delegate_generic_class *object, Params ... args)
	{
		delegate_mfp *_this = reinterpret_cast<delegate_mfp *>(object);
		using mfptype = ReturnType(FunctionClass::*)(Params...);
		mfptype &mfp = *reinterpret_cast<mfptype *>(&_this->m_rawdata);
		return (reinterpret_cast<FunctionClass *>(_this->m_realobject)->*mfp)(std::forward<Params>(args)...);
	}


	// helper to convert a function of a given type to a generic function, forcing template
	// instantiation to match the source type
	template <typename SourceType>
	static delegate_generic_function make_generic(SourceType funcptr)
	{
		return reinterpret_cast<delegate_generic_function>(funcptr);
	}


	struct raw_mfp_data
	{
#if defined(__INTEL_COMPILER) && defined(_M_X64) // needed for "Intel(R) C++ Intel(R) 64 Compiler XE for applications running on Intel(R) 64, Version 14.0.2.176 Build 20140130" at least
		int data[((sizeof(void *) + 4 * sizeof(int)) + (sizeof(int) - 1)) / sizeof(int)];
#else // all other cases - for MSVC maximum size is one pointer, plus 3 ints; all other implementations seem to be smaller
		int data[((sizeof(void *) + 3 * sizeof(int)) + (sizeof(int) - 1)) / sizeof(int)];
#endif
		bool operator==(const raw_mfp_data &rhs) const { return (memcmp(data, rhs.data, sizeof(data)) == 0); }
	};

	// internal state
	raw_mfp_data                m_rawdata;          // raw buffer to hold the copy of the function pointer
	delegate_generic_class *    m_realobject;       // pointer to the object used for calling
	delegate_generic_function   m_stubfunction;     // pointer to our matching stub function

	static const raw_mfp_data   s_null_mfp;         // nullptr mfp
};

#elif (USE_DELEGATE_TYPE == DELEGATE_TYPE_INTERNAL)

// ======================> delegate_mfp

// struct describing the contents of a member function pointer
class delegate_mfp
{
public:
	// default constructor
	delegate_mfp() = default;

	// copy constructor
	delegate_mfp(const delegate_mfp &src) = default;

	// construct from any member function pointer
	template <typename MemberFunctionType, class MemberFunctionClass, typename ReturnType, typename StaticFunctionType>
	delegate_mfp(MemberFunctionType mfp, MemberFunctionClass *, ReturnType *, StaticFunctionType)
	{
		static_assert(sizeof(mfp) == sizeof(*this), "Unsupported member function pointer size");
		*reinterpret_cast<MemberFunctionType *>(this) = mfp;
	}

	// comparison helpers
	bool operator==(const delegate_mfp &rhs) const { return (m_function == rhs.m_function) && (m_this_delta == rhs.m_this_delta); }
	bool isnull() const { return !m_function && !m_this_delta; }

	// getters
	static delegate_generic_class *real_object(delegate_generic_class *original) { return original; }

	// binding helper
	template <typename FunctionType>
	void update_after_bind(FunctionType &funcptr, delegate_generic_class *&object)
	{
		funcptr = reinterpret_cast<FunctionType>(convert_to_generic(object));
	}

private:
	// extract the generic function and adjust the object pointer
	delegate_generic_function convert_to_generic(delegate_generic_class *&object) const;

	// actual state
	uintptr_t               m_function = 0;     // first item can be one of two things:
												//    if even, it's a pointer to the function
												//    if odd, it's the byte offset into the vtable
	int                     m_this_delta = 0;   // delta to apply to the 'this' pointer
};

#elif (USE_DELEGATE_TYPE == DELEGATE_TYPE_MSVC)

// ======================> delegate_mfp

// struct describing the contents of a member function pointer
class delegate_mfp
{
	struct single_base_equiv { delegate_generic_function p; };
	struct multi_base_equiv { delegate_generic_function p; int o; };

public:
	// default constructor
	delegate_mfp() = default;

	// copy constructor
	delegate_mfp(const delegate_mfp &src) = default;

	// construct from any member function pointer
	template <typename MemberFunctionType, class MemberFunctionClass, typename ReturnType, typename StaticFunctionType>
	delegate_mfp(MemberFunctionType mfp, MemberFunctionClass *, ReturnType *, StaticFunctionType)
	{
		static_assert((sizeof(mfp) == sizeof(single_base_equiv)) || (sizeof(mfp) == sizeof(multi_base_equiv)), "Unsupported member function pointer size");
		m_size = sizeof(mfp);
		*reinterpret_cast<MemberFunctionType *>(this) = mfp;
	}

	// comparison helpers
	bool operator==(const delegate_mfp &rhs) const { return m_function == rhs.m_function; }
	bool isnull() const { return !m_function; }

	// getters
	static delegate_generic_class *real_object(delegate_generic_class *original) { return original; }

	// binding helper
	template <typename FunctionType>
	void update_after_bind(FunctionType &funcptr, delegate_generic_class *&object)
	{
		funcptr = reinterpret_cast<FunctionType>(m_function);
		if (m_size > sizeof(single_base_equiv))
			object = reinterpret_cast<delegate_generic_class *>(reinterpret_cast<std::uint8_t *>(object) + m_this_delta);
	}

private:
	// actual state
	uintptr_t   m_function = 0;     // pointer to function or non-virtual thunk for vtable index
	int         m_this_delta = 0;   // delta to apply to the 'this' pointer for multiple inheritance

	int         m_dummy1 = 0;
	int         m_dummy2 = 0;

	unsigned    m_size = 0;
};

#endif

} // namespace util::detail



// ======================> delegate_late_bind

// simple polymorphic class that must be mixed into any object that is late-bound
class delegate_late_bind
{
public:
	delegate_late_bind() { }
	virtual ~delegate_late_bind() { }
};


// ======================> binding_type_exception

// exception that is thrown when a bind fails the dynamic_cast
class binding_type_exception : public std::exception
{
public:
	binding_type_exception(const std::type_info &target_type, const std::type_info &actual_type)
		: m_target_type(target_type), m_actual_type(actual_type)
	{ }
	const std::type_info &m_target_type;
	const std::type_info &m_actual_type;
};


//**************************************************************************
//  COMMON DELEGATE BASE CLASS
//**************************************************************************

// ======================> delegate_base

// general delegate class template supporting up to 5 parameters
template <typename ReturnType, typename... Params>
class delegate_base
{
public:
	// define our traits
	template <class FunctionClass> using traits = util::detail::delegate_traits<FunctionClass, ReturnType, Params...>;
	using generic_static_func = typename traits<util::detail::delegate_generic_class>::static_func_type;
	typedef MEMBER_ABI generic_static_func generic_member_func;

	// generic constructor
	delegate_base() = default;

	// copy constructor
	delegate_base(const delegate_base &src)
		: m_function(src.m_function)
		, m_latebinder(src.m_latebinder)
		, m_raw_function(src.m_raw_function)
		, m_raw_mfp(src.m_raw_mfp)
	{
		bind(src.object());
	}

	// copy constructor with late bind
	delegate_base(const delegate_base &src, delegate_late_bind &object)
		: m_function(src.m_function)
		, m_latebinder(src.m_latebinder)
		, m_raw_function(src.m_raw_function)
		, m_raw_mfp(src.m_raw_mfp)
	{
		late_bind(object);
	}

	// construct from member function with object pointer
	template <class FunctionClass>
	delegate_base(typename traits<FunctionClass>::member_func_type funcptr, FunctionClass *object)
		: m_latebinder(&late_bind_helper<FunctionClass>)
		, m_raw_mfp(funcptr, object, static_cast<ReturnType *>(nullptr), static_cast<generic_static_func>(nullptr))
	{
		bind(reinterpret_cast<util::detail::delegate_generic_class *>(object));
	}

	template <class FunctionClass>
	delegate_base(typename traits<FunctionClass>::const_member_func_type funcptr, FunctionClass *object)
		: m_latebinder(&late_bind_helper<FunctionClass>)
		, m_raw_mfp(funcptr, object, static_cast<ReturnType *>(nullptr), static_cast<generic_static_func>(nullptr))
	{
		bind(reinterpret_cast<util::detail::delegate_generic_class *>(object));
	}

	// construct from static reference function with object reference
	template <class FunctionClass>
	delegate_base(typename traits<FunctionClass>::static_ref_func_type funcptr, FunctionClass *object)
		: m_function(reinterpret_cast<generic_static_func>(funcptr))
		, m_latebinder(&late_bind_helper<FunctionClass>)
		, m_raw_function(reinterpret_cast<generic_static_func>(funcptr))
	{
		bind(reinterpret_cast<util::detail::delegate_generic_class *>(object));
	}

	// copy operator
	delegate_base &operator=(const delegate_base &src)
	{
		if (this != &src)
		{
			m_function = src.m_function;
			m_object = nullptr;
			m_latebinder = src.m_latebinder;
			m_raw_function = src.m_raw_function;
			m_raw_mfp = src.m_raw_mfp;

			bind(src.object());
		}
		return *this;
	}

	// comparison helper
	bool operator==(const delegate_base &rhs) const
	{
		return (m_raw_function == rhs.m_raw_function) && (object() == rhs.object()) && (m_raw_mfp == rhs.m_raw_mfp);
	}


	// call the function
	ReturnType operator()(Params... args) const
	{
		if (is_mfp() && (HAS_DIFFERENT_ABI))
			return (*reinterpret_cast<generic_member_func>(m_function))(m_object, std::forward<Params>(args)...);
		else
			return (*m_function)(m_object, std::forward<Params>(args)...);
	}

	// getters
	bool has_object() const { return object() != nullptr; }

	// helpers
	bool isnull() const { return !m_raw_function && m_raw_mfp.isnull(); }
	bool is_mfp() const { return !m_raw_mfp.isnull(); }

	// late binding
	void late_bind(delegate_late_bind &object)
	{
		if (m_latebinder)
			bind((*m_latebinder)(object));
	}

protected:
	// return the actual object (not the one we use for calling)
	util::detail::delegate_generic_class *object() const { return is_mfp() ? m_raw_mfp.real_object(m_object) : m_object; }

	// late binding function
	using late_bind_func = util::detail::delegate_generic_class*(*)(delegate_late_bind &object);

	// late binding helper
	template <class FunctionClass>
	static util::detail::delegate_generic_class *late_bind_helper(delegate_late_bind &object)
	{
		FunctionClass *result = dynamic_cast<FunctionClass *>(&object);
		if (!result)
			throw binding_type_exception(typeid(FunctionClass), typeid(object));

		return reinterpret_cast<util::detail::delegate_generic_class *>(result);
	}

	// bind the actual object
	void bind(util::detail::delegate_generic_class *object)
	{
		m_object = object;

		// if we're wrapping a member function pointer, handle special stuff
		if (m_object && is_mfp())
			m_raw_mfp.update_after_bind(m_function, m_object);
	}

	// internal state
	generic_static_func                     m_function = nullptr;       // resolved static function pointer
	util::detail::delegate_generic_class *  m_object = nullptr;         // resolved object to the post-cast object
	late_bind_func                          m_latebinder = nullptr;     // late binding helper
	generic_static_func                     m_raw_function = nullptr;   // raw static function pointer
	util::detail::delegate_mfp              m_raw_mfp;                  // raw member function pointer
};



//**************************************************************************
//  NATURAL SYNTAX
//**************************************************************************

// declare the base template
template <typename Signature> class delegate;

template <typename ReturnType, typename... Params>
class delegate<ReturnType (Params...)> : public delegate_base<ReturnType, Params...>
{
private:
	using basetype = delegate_base<ReturnType, Params...>;
	using functoid_setter = void (*)(delegate &);

	template <typename T> struct functoid_type_unwrap { using type = std::remove_reference_t<T>; };
	template <typename T> struct functoid_type_unwrap<std::reference_wrapper<T> > { using type = typename functoid_type_unwrap<T>::type; };
	template <typename T> using unwrapped_functoid_t = typename functoid_type_unwrap<std::remove_cv_t<std::remove_reference_t<T> > >::type;

	template <typename T> static constexpr bool matching_non_const_call(T &&) { return false; }
	template <typename T> static constexpr bool matching_non_const_call(ReturnType (T::*)(Params...)) { return true; }
	template <typename T> static constexpr bool matching_const_call(T &&) { return false; }
	template <typename T> static constexpr bool matching_const_call(ReturnType (T::*)(Params...) const) { return true; }

	template <typename T> static T *unwrap_functoid(T *functoid) { return functoid; }
	template <typename T> static T *unwrap_functoid(std::reference_wrapper<T> *functoid) { return &functoid->get(); }

	template <typename T>
	unwrapped_functoid_t<T> *unwrap_functoid() noexcept
	{
		return unwrap_functoid(std::any_cast<std::remove_cv_t<std::remove_reference_t<T> > >(&m_functoid));
	}

	template <typename T>
	static functoid_setter make_functoid_setter()
	{
		if constexpr (matching_non_const_call(&unwrapped_functoid_t<T>::operator()))
		{
			return
					[] (delegate &obj)
					{
						obj.basetype::operator=(
								basetype(
									static_cast<ReturnType (unwrapped_functoid_t<T>::*)(Params...)>(&unwrapped_functoid_t<T>::operator()),
									obj.unwrap_functoid<T>()));
					};
		}
		else if constexpr (matching_const_call(&unwrapped_functoid_t<T>::operator()))
		{
			return
					[] (delegate &obj)
					{
						obj.basetype::operator=(
								basetype(
									static_cast<ReturnType (unwrapped_functoid_t<T>::*)(Params...) const>(&unwrapped_functoid_t<T>::operator()),
									obj.unwrap_functoid<T>()));
					};
		}
		else
		{
			return
					[] (delegate &obj)
					{
						obj.basetype::operator=(
								basetype(
									[] (unwrapped_functoid_t<T> &f, Params... args) { return ReturnType(f(std::forward<Params>(args)...)); },
									obj.unwrap_functoid<T>()));
					};
		}
	}

	std::any m_functoid;
	functoid_setter m_set_functoid = nullptr;

protected:
	template <class FunctionClass> using traits = typename basetype::template traits<FunctionClass>;
	template <class FunctionClass> using member_func_type = typename traits<FunctionClass>::member_func_type;
	template <class FunctionClass> using const_member_func_type = typename traits<FunctionClass>::const_member_func_type;
	template <class FunctionClass> using static_ref_func_type = typename traits<FunctionClass>::static_ref_func_type;

	template <typename T> using suitable_functoid = std::bool_constant<std::is_convertible_v<std::invoke_result_t<T, Params...>, ReturnType> || std::is_same_v<std::void_t<std::invoke_result_t<T, Params...> >, ReturnType> >;

public:
	delegate() : basetype() { }

	delegate(delegate const &src)
		: basetype(src.m_functoid.has_value() ? static_cast<const basetype &>(basetype()) : src)
		, m_functoid(src.m_functoid)
		, m_set_functoid(src.m_set_functoid)
	{
		if (m_functoid.has_value())
			m_set_functoid(*this);
	}

	delegate(delegate &src)
		: delegate(const_cast<delegate const &>(src))
	{
	}

	delegate(delegate &&src)
		: basetype(src.m_functoid.has_value() ? basetype() : std::move(src))
		, m_functoid(std::move(src.m_functoid))
		, m_set_functoid(std::move(src.m_set_functoid))
	{
		if (m_functoid.has_value())
			m_set_functoid(*this);
	}

	delegate(delegate const &src, delegate_late_bind &object)
		: basetype(src.m_functoid.has_value() ? basetype() : basetype(src, object))
		, m_functoid(src.m_functoid)
		, m_set_functoid(src.m_set_functoid)
	{
		if (m_functoid.has_value())
			m_set_functoid(*this);
	}

	template <class FunctionClass> delegate(member_func_type<FunctionClass> funcptr, FunctionClass *object) : basetype(funcptr, object) { }
	template <class FunctionClass> delegate(const_member_func_type<FunctionClass> funcptr, FunctionClass *object) : basetype(funcptr, object) { }
	template <class FunctionClass> delegate(static_ref_func_type<FunctionClass> funcptr, FunctionClass *object) : basetype(funcptr, object) { }

	template <typename T>
	explicit delegate(T &&functoid, std::enable_if_t<suitable_functoid<T>::value, int> = 0)
		: basetype()
		, m_functoid(std::forward<T>(functoid))
		, m_set_functoid(make_functoid_setter<T>())
	{
		m_set_functoid(*this);
	}

	delegate &operator=(std::nullptr_t) noexcept
	{
		reset();
		return *this;
	}

	delegate &operator=(delegate const &src)
	{
		m_functoid = src.m_functoid;
		m_set_functoid = src.m_set_functoid;
		if (m_functoid.has_value())
			m_set_functoid(*this);
		else
			basetype::operator=(src);
		return *this;
	}

	delegate &operator=(delegate &&src)
	{
		m_functoid = std::move(src.m_functoid);
		m_set_functoid = std::move(src.m_set_functoid);
		if (m_functoid.has_value())
			m_set_functoid(*this);
		else
			basetype::operator=(std::move(src));
		return *this;
	}

	void reset() noexcept
	{
		basetype::operator=(basetype());
		m_functoid.reset();
		m_set_functoid = nullptr;
	}
};

#endif // MAME_UTIL_DELEGATE_H
