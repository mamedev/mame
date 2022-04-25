// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Couriersud,Miodrag Milanovic,Vas Crabb
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

        https://www.codeproject.com/Articles/11015/The-Impossibly-Fast-C-Delegates

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

    The "Itanium" version of delegates makes use of the internal
    structure of member function pointers in order to convert them at
    binding time into simple static function pointers. This only works
    on platforms where object->func(p1, p2) is equivalent in calling
    convention to func(object, p1, p2).

    Pros:
        * as fast as a standard function call in static and member cases
        * no stub functions or double-hops needed

    Cons:
        * requires internal knowledge of the member function pointer
        * only works for two popular variants of the Itanium C++ ABI

    --------------------------------------------------------------------

    The "MSVC" version of delegates makes use of the internal structure
    of member function pointers in order to convert them at binding time
    into simple static function pointers. This only works on platforms
    where object->func(p1, p2) is equivalent in calling convention to
    func(object, p1, p2).

    Pros:
        * as fast as a standard function call in static and non-virtual
          member cases
        * no stub functions needed

    Cons:
        * requires internal knowledge of the member function pointer
        * only works works with MSVC ABI, and not on 32-bit x86
        * does not work for classes with virtual bases
        * structure return does not work with member function pointers
        * virtual member function lookup cannot be done in advance

    --------------------------------------------------------------------

    Further reading:

        * http://itanium-cxx-abi.github.io/cxx-abi/abi.html#member-pointers
          Formal specification for the most common member function pointer
          implementations.

        * https://www.codeproject.com/Articles/7150/Member-Function-Pointers-and-the-Fastest-Possible
          Discusses many member function pointer implementations.  Based
          on reverse-engineering, so not entirely accurate.  In particular,
          various fields are incorrectly assumed to be int-sized which is
          not true in the general case.

        * https://devblogs.microsoft.com/oldnewthing/20040209-00/?p=40713
          Describes the MSVC implementation of pointers to member
          functions for classes with single or multiple inheritance.  Does
          not mention the additional variants for virtual or unknown
          inheritance.  Incorrectly states that the "this" pointer
          displacement is a size_t when in reality it is an int (important
          for 64-bit architectures).

***************************************************************************/
#ifndef MAME_LIB_UTIL_DELEGATE_H
#define MAME_LIB_UTIL_DELEGATE_H

#pragma once

#include "abi.h"

#include <any>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>


//**************************************************************************
//  MACROS
//**************************************************************************

// types of delegates supported
#define MAME_DELEGATE_TYPE_COMPATIBLE 0
#define MAME_DELEGATE_TYPE_ITANIUM 1
#define MAME_DELEGATE_TYPE_MSVC 2

// select which one we will be using
#if defined(MAME_DELEGATE_FORCE_COMPATIBLE)
	#define MAME_DELEGATE_USE_TYPE MAME_DELEGATE_TYPE_COMPATIBLE
#elif defined(__GNUC__)
	// 32bit MINGW asks for different convention
	#if defined(__MINGW32__) && !defined(__x86_64__) && defined(__i386__)
		#define MAME_DELEGATE_USE_TYPE MAME_DELEGATE_TYPE_COMPATIBLE
		//#define MAME_DELEGATE_USE_TYPE MAME_DELEGATE_TYPE_ITANIUM
		//#define MAME_DELEGATE_DIFFERENT_MEMBER_ABI 1
	#elif defined(__clang__) && defined(__i386__) && defined(_WIN32)
		#define MAME_DELEGATE_USE_TYPE MAME_DELEGATE_TYPE_COMPATIBLE
	#else
		#define MAME_DELEGATE_USE_TYPE MAME_DELEGATE_TYPE_ITANIUM
		#define MAME_DELEGATE_DIFFERENT_MEMBER_ABI 0
	#endif
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))
	#define MAME_DELEGATE_DIFFERENT_MEMBER_ABI 0
	#define MAME_DELEGATE_USE_TYPE MAME_DELEGATE_TYPE_MSVC
#else
	#define MAME_DELEGATE_USE_TYPE MAME_DELEGATE_TYPE_COMPATIBLE
#endif

#if MAME_DELEGATE_USE_TYPE == MAME_DELEGATE_TYPE_COMPATIBLE
	#define MAME_DELEGATE_DIFFERENT_MEMBER_ABI 0
#endif


/// \brief Base for objects used with late binding
///
/// Default polymorphic class used as base for objects that can be bound
/// to after the target function has already been set.
class delegate_late_bind
{
public:
	virtual ~delegate_late_bind() = default;
};


/// \brief Inappropriate late bind object error
///
/// Thrown as an exception if the object supplied for late binding
/// cannot be cast to the target type for the delegate's function.
class binding_type_exception : public std::bad_cast
{
public:
	binding_type_exception(std::type_info const &target_type, std::type_info const &actual_type);

	virtual char const *what() const noexcept override;

	std::type_info const &target_type() const noexcept { return *m_target_type; }
	std::type_info const &actual_type() const noexcept { return *m_actual_type; }

private:
	std::string m_what;
	std::type_info const *m_target_type;
	std::type_info const *m_actual_type;
};



namespace util::detail {

//**************************************************************************
//  HELPER CLASSES
//**************************************************************************

// generic function type
using delegate_generic_function = void(*)();


// ======================> generic_class

// define a dummy generic class that is just straight single-inheritance
#ifdef _MSC_VER
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



/// \brief Maximally compatible member function pointer wrapper
///
/// Instantiates a static member function template on construction as
/// an adaptor thunk to call the supplied member function with the
/// supplied object.  Adds one layer of indirection to calls.
///
/// This implementation requires the representation of a null member
/// function pointer to be all zeroes.
class delegate_mfp_compatible
{
public:
	// default constructor
	delegate_mfp_compatible()
		: m_rawdata(s_null_mfp)
		, m_realobject(nullptr)
		, m_stubfunction(nullptr)
	{ }

	// copy constructor
	delegate_mfp_compatible(const delegate_mfp_compatible &src) = default;

	// construct from any member function pointer
	template <typename MemberFunctionType, class MemberFunctionClass, typename ReturnType, typename StaticFunctionType>
	delegate_mfp_compatible(MemberFunctionType mfp, MemberFunctionClass *, ReturnType *, StaticFunctionType)
		: m_rawdata(s_null_mfp)
		, m_realobject(nullptr)
		, m_stubfunction(make_generic<StaticFunctionType>(&delegate_mfp_compatible::method_stub<MemberFunctionClass, ReturnType>))
	{
		static_assert(sizeof(mfp) <= sizeof(m_rawdata), "Unsupported member function pointer size");
		*reinterpret_cast<MemberFunctionType *>(&m_rawdata) = mfp;
	}

	// comparison helpers
	bool operator==(const delegate_mfp_compatible &rhs) const { return m_rawdata == rhs.m_rawdata; }
	bool isnull() const { return m_rawdata == s_null_mfp; }

	// getters
	delegate_generic_class *real_object(delegate_generic_class *original) const
	{
		return m_realobject;
	}

	// binding helpers
	template <typename FunctionType>
	void update_after_bind(FunctionType &funcptr, delegate_generic_class *&object);

	template <typename FunctionType>
	void update_after_copy(FunctionType &funcptr, delegate_generic_class *&object);

private:
	// helper stubs for calling encased member function pointers
	template <class FunctionClass, typename ReturnType, typename... Params>
	static ReturnType method_stub(delegate_generic_class *object, Params ... args);

	// helper to convert a function of a given type to a generic function, forcing template
	// instantiation to match the source type
	template <typename SourceType>
	static delegate_generic_function make_generic(SourceType funcptr)
	{
		return reinterpret_cast<delegate_generic_function>(funcptr);
	}

	// FIXME: not properly aligned for storing pointers
	struct raw_mfp_data
	{
#if defined(__INTEL_COMPILER) && defined(_M_X64) // needed for "Intel(R) C++ Intel(R) 64 Compiler XE for applications running on Intel(R) 64, Version 14.0.2.176 Build 20140130" at least
		int data[((sizeof(void *) + 4 * sizeof(int)) + (sizeof(int) - 1)) / sizeof(int)];
#else // all other cases - for MSVC maximum size is one pointer, plus 3 ints; all other implementations seem to be smaller
		int data[((sizeof(void *) + 3 * sizeof(int)) + (sizeof(int) - 1)) / sizeof(int)];
#endif
		bool operator==(const raw_mfp_data &rhs) const { return !std::memcmp(data, rhs.data, sizeof(data)); }
	};

	// internal state
	raw_mfp_data                m_rawdata;          // raw buffer to hold the copy of the function pointer
	delegate_generic_class *    m_realobject;       // pointer to the object used for calling
	delegate_generic_function   m_stubfunction;     // pointer to our matching stub function

	static const raw_mfp_data   s_null_mfp;         // nullptr mfp
};


template <typename FunctionType>
void delegate_mfp_compatible::update_after_bind(FunctionType &funcptr, delegate_generic_class *&object)
{
	m_realobject = object;
	object = reinterpret_cast<delegate_generic_class *>(this);
	funcptr = reinterpret_cast<FunctionType>(m_stubfunction);
}


template <typename FunctionType>
void delegate_mfp_compatible::update_after_copy(FunctionType &funcptr, delegate_generic_class *&object)
{
	assert(reinterpret_cast<FunctionType>(m_stubfunction) == funcptr);
	object = reinterpret_cast<delegate_generic_class *>(this);
}


template <class FunctionClass, typename ReturnType, typename... Params>
ReturnType delegate_mfp_compatible::method_stub(delegate_generic_class *object, Params ... args)
{
	using mfptype = ReturnType(FunctionClass::*)(Params...);
	delegate_mfp_compatible &_this = *reinterpret_cast<delegate_mfp_compatible *>(object);
	mfptype &mfp = *reinterpret_cast<mfptype *>(&_this.m_rawdata);
	return (reinterpret_cast<FunctionClass *>(_this.m_realobject)->*mfp)(std::forward<Params>(args)...);
}



/// \brief Itanium C++ ABI member function pointer wrapper
///
/// Supports the two most popular pointer to member function
/// implementations described in the Itanium C++ ABI.  Both of these
/// consist of a pointer followed by a ptrdiff_t.
///
/// The first variant is used when member the least significant bit of a
/// member function pointer need never be set and vtable entry offsets
/// are guaranteed to be even numbers of bytes.  If the pointer is even,
/// it is a conventional function pointer to the member function.  If
/// the pointer is odd, it is a byte offset into the vtable plus one.
/// The ptrdiff_t is a byte offset to add to the this pointer.  A null
/// member function pointer is represented by setting the pointer to a
/// null pointer.
///
/// The second variant is used when the least significant bit of a
/// pointer to a member function may need to be set or it may not be
/// possible to distinguish between a vtable offset and a null pointer.
/// (This is the case for ARM where the least significant bit of a
/// pointer to a function is set if the function starts in Thumb mode.)
/// If the least significant bit of the ptrdiff_t is clear, the pointer
/// is a conventional function pointer to the member function.  If the
/// least significant bit of the ptrdiff_t is set, the pointer is a byte
/// offset into the vtable.  The ptrdiff_t must be shifted right one bit
/// position to make a byte offset to add to the this pointer.  A null
/// member function pointer is represented by setting the pointer to a
/// null pointer and clearing the least significant bit of the
/// ptrdiff_t.
class delegate_mfp_itanium
{
public:
	// default constructor
	delegate_mfp_itanium() = default;

	// copy constructor
	delegate_mfp_itanium(const delegate_mfp_itanium &src) = default;

	// construct from any member function pointer
	template <typename MemberFunctionType, class MemberFunctionClass, typename ReturnType, typename StaticFunctionType>
	delegate_mfp_itanium(MemberFunctionType mfp, MemberFunctionClass *, ReturnType *, StaticFunctionType)
	{
		static_assert(sizeof(mfp) == sizeof(*this), "Unsupported member function pointer size");
		*reinterpret_cast<MemberFunctionType *>(this) = mfp;
	}

	// comparison helpers
	bool operator==(const delegate_mfp_itanium &rhs) const
	{
		return (isnull() && rhs.isnull()) || ((m_function == rhs.m_function) && (m_this_delta == rhs.m_this_delta));
	}

	bool isnull() const
	{
		if (MAME_ABI_CXX_ITANIUM_MFP_TYPE == MAME_ABI_CXX_ITANIUM_MFP_ARM)
			return !reinterpret_cast<void (*)()>(m_function) && !(m_this_delta & 1);
		else
			return !reinterpret_cast<void (*)()>(m_function);
	}

	// getters
	static delegate_generic_class *real_object(delegate_generic_class *original)
	{
		return original;
	}

	// binding helpers
	template <typename FunctionType>
	void update_after_bind(FunctionType &funcptr, delegate_generic_class *&object)
	{
		funcptr = reinterpret_cast<FunctionType>(convert_to_generic(object));
	}

	template <typename FunctionType>
	void update_after_copy(FunctionType &funcptr, delegate_generic_class *&object)
	{
	}

private:
	// extract the generic function and adjust the object pointer
	delegate_generic_function convert_to_generic(delegate_generic_class *&object) const;

	// actual state
	uintptr_t   m_function = reinterpret_cast<uintptr_t>(static_cast<void (*)()>(nullptr)); // function pointer or vtable offset
	ptrdiff_t   m_this_delta = 0;                                                           // delta to apply to the 'this' pointer
};



/// \brief MSVC member function pointer wrapper
///
/// MSVC uses space optimisation.  A member function pointer is a
/// conventional function pointer followed by zero to three int values,
/// depending on whether the class has single, multiple, virtual or
/// unknown inheritance of base classes.  The function pointer is always
/// a conventional function pointer (a thunk is used to call virtual
/// member functions through the vtable).
///
/// If present, the first int value is a byte offset to add to the this
/// pointer before calling the function.
///
/// For the virtual inheritance case, the offset to the vtable pointer
/// from the location the this pointer points to must be known by the
/// compiler when the member function pointer is called.  The second int
/// value is a byte offset into the vtable to an int value containing an
/// additional byte offset to add to the this pointer.
///
/// For the unknown inheritance case, the second int value is a byte
/// offset add to the this pointer to obtain a pointer to the vtable
/// pointer, or undefined if not required.  If the third int value is
/// not zero, it is a byte offset into the vtable to an int value
/// containing an additional byte offset to add to the this pointer.
///
/// It is not possible to support the virtual inheritance case without
/// some way of obtaining the offset to the vtable pointer.
class delegate_mfp_msvc
{
	struct single_base_equiv { delegate_generic_function fptr; };
	struct multi_base_equiv { delegate_generic_function fptr; int thisdisp; };
	struct unknown_base_equiv { delegate_generic_function fptr; int thisdisp, vptrdisp, vtdisp; };

public:
	// default constructor
	delegate_mfp_msvc() = default;

	// copy constructor
	delegate_mfp_msvc(const delegate_mfp_msvc &src) = default;

	// construct from any member function pointer
	template <typename MemberFunctionType, class MemberFunctionClass, typename ReturnType, typename StaticFunctionType>
	delegate_mfp_msvc(MemberFunctionType mfp, MemberFunctionClass *, ReturnType *, StaticFunctionType)
	{
		// FIXME: this doesn't actually catch the unsupported virtual inheritance case on 64-bit targets
		// alignment of the pointer means sizeof gives the same value for multiple inheritance and virtual inheritance cases
		static_assert(
				(sizeof(mfp) == sizeof(single_base_equiv)) || (sizeof(mfp) == sizeof(multi_base_equiv)) || (sizeof(mfp) == sizeof(unknown_base_equiv)),
				"Unsupported member function pointer size");
		static_assert(sizeof(mfp) <= sizeof(*this), "Member function pointer is too large to support");
		*reinterpret_cast<MemberFunctionType *>(this) = mfp;
		m_size = sizeof(mfp);
	}

	// comparison helpers
	bool operator==(const delegate_mfp_msvc &rhs) const
	{
		if (m_function != rhs.m_function)
		{
			return false;
		}
		else if (sizeof(single_base_equiv) == m_size)
		{
			return (sizeof(single_base_equiv) == rhs.m_size) || (!rhs.m_this_delta && ((sizeof(multi_base_equiv) == rhs.m_size) || !rhs.m_vt_index));
		}
		else if (sizeof(multi_base_equiv) == m_size)
		{
			if (sizeof(unknown_base_equiv) == rhs.m_size)
				return (m_this_delta == rhs.m_this_delta) && !rhs.m_vt_index;
			else
				return (sizeof(single_base_equiv) == rhs.m_size) ? !m_this_delta : (m_this_delta == rhs.m_this_delta);
		}
		else if (sizeof(unknown_base_equiv) == rhs.m_size)
		{
			return (m_this_delta == rhs.m_this_delta) && (m_vt_index == rhs.m_vt_index) && (!m_vt_index || (m_vptr_offs == rhs.m_vptr_offs));
		}
		else
		{
			return !m_vt_index && ((sizeof(multi_base_equiv) == rhs.m_size) ? (m_this_delta == rhs.m_this_delta) : !m_this_delta);
		}
	}

	bool isnull() const
	{
		return !reinterpret_cast<void (*)()>(m_function);
	}

	// getters
	static delegate_generic_class *real_object(delegate_generic_class *original) { return original; }

	// binding helpers
	template <typename FunctionType>
	void update_after_bind(FunctionType &funcptr, delegate_generic_class *&object)
	{
		funcptr = reinterpret_cast<FunctionType>(adjust_this_pointer(object));
	}

	template <typename FunctionType>
	void update_after_copy(FunctionType &funcptr, delegate_generic_class *&object)
	{
	}

private:
	// adjust the object pointer and bypass thunks
	delegate_generic_function adjust_this_pointer(delegate_generic_class *&object) const;

	// actual state
	uintptr_t   m_function = 0;     // pointer to function or non-virtual thunk for virtual function call
	int         m_this_delta = 0;   // delta to apply to the 'this' pointer for multiple inheritance
	int         m_vptr_offs = 0;    // offset to apply to this pointer to obtain pointer to vptr
	int         m_vt_index = 0;     // offset into vtable to additional delta to apply to the 'this' pointer

	unsigned    m_size = 0;         // overall size of the pointer to member function representation
};



#if MAME_DELEGATE_USE_TYPE == MAME_DELEGATE_TYPE_COMPATIBLE

template <typename ReturnType>
struct delegate_mfp { using type = delegate_mfp_compatible; };

#elif MAME_DELEGATE_USE_TYPE == MAME_DELEGATE_TYPE_ITANIUM

template <typename ReturnType>
struct delegate_mfp { using type = delegate_mfp_itanium; };

#elif MAME_DELEGATE_USE_TYPE == MAME_DELEGATE_TYPE_MSVC

/// \brief Determine whether a type is returned conventionally
///
/// Under the MSVC C++ ABI with the Microsoft calling convention for
/// x86-64 or AArch64, the calling convention for member functions is
/// not quite the same as a free function with the "this" pointer as the
/// first parameter.
///
/// Conventionally, structure and union values can be returned in
/// registers if they are small enough and are aggregates (trivially
/// constructible, destructible, copyable and assignable).  On x86-64,
/// if the value cannot be returned in registers, the pointer to the
/// area for the return value is conventionally passed in RCX and
/// explicit parameters are shifted by one position.  On AArch64, if the
/// value cannot be returned in registers, the pointer to the area for
/// the return value is passed in X8 (explicit parameters do not need to
/// be shifted).
///
/// For member functions, structure and union types are never returned
/// in registers, and the pointer to the area for the return value is
/// passed differently for structures and unions.  When a structure or
/// union is to be returned, a pointer to the area for the return value
/// is effectively passed as a second implicit parameter.  On x86-64,
/// the "this" pointer is passed in RCX and the pointer to the area for
/// the return value is passed in RDX; on AArch64, the "this" pointer is
/// passed in X0 and the pointer to the area for the return value is
/// passed in X1.  Explicit parameters are shifted an additional
/// position to allow for the second implicit parameter.
///
/// Note that pointer types are returned conventionally from member
/// functions even when they're too large to return in registers (e.g. a
/// pointer to a function member of a class with unknown inheritance).
///
/// Because of this, we may need to use the #delegate_mfp_compatible
/// class to generate adaptor thunks depending on the return type.  This
/// trait doesn't need to reliably be true for types that are returned
/// conventionally from member functions; it only needs to reliably be
/// false for types that aren't.  Incorrectly yielding true will result
/// in incorrect behaviour while incorrectly yielding false will just
/// cause increased overhead (both compile-time and run-time).
template <typename ReturnType>
using delegate_mfp_conventional_return = std::bool_constant<
		std::is_void_v<ReturnType> ||
		std::is_scalar_v<ReturnType> ||
		std::is_reference_v<ReturnType> >;

template <typename ReturnType, typename Enable = void>
struct delegate_mfp;

template <typename ReturnType>
struct delegate_mfp<ReturnType, std::enable_if_t<delegate_mfp_conventional_return<ReturnType>::value> > { using type = delegate_mfp_msvc; };

template <typename ReturnType>
struct delegate_mfp<ReturnType, std::enable_if_t<!delegate_mfp_conventional_return<ReturnType>::value> > { using type = delegate_mfp_compatible; };

#endif

template <typename ReturnType> using delegate_mfp_t = typename delegate_mfp<ReturnType>::type;



/// \brief Helper class for generating late bind functions
///
/// Members of this class don't depend on the delegate's signature.
/// Keeping them here reduces the number of template instantiations as
/// you'll only need one late bind helper for each class used for late
/// binding, not for each class for each delegate signature.
template <class LateBindBase>
class delegate_late_bind_helper
{
public:
	// make it default constructible and copyable
	delegate_late_bind_helper() = default;
	delegate_late_bind_helper(delegate_late_bind_helper const &) = default;
	delegate_late_bind_helper(delegate_late_bind_helper &&) = default;
	delegate_late_bind_helper &operator=(delegate_late_bind_helper const &) = default;
	delegate_late_bind_helper &operator=(delegate_late_bind_helper &&) = default;

	template <class FunctionClass>
	delegate_late_bind_helper(FunctionClass *)
		: m_latebinder(&delegate_late_bind_helper::late_bind_helper<FunctionClass>)
	{
	}

	delegate_generic_class *operator()(LateBindBase &object) { return m_latebinder(object); }

	explicit operator bool() const noexcept { return bool(m_latebinder); }

private:
	using late_bind_func = delegate_generic_class*(*)(LateBindBase &object);

	template <class FunctionClass> static delegate_generic_class *late_bind_helper(LateBindBase &object);

	late_bind_func  m_latebinder = nullptr;
};


template <class LateBindBase>
template <class FunctionClass>
delegate_generic_class *delegate_late_bind_helper<LateBindBase>::late_bind_helper(LateBindBase &object)
{
	FunctionClass *result = dynamic_cast<FunctionClass *>(&object);
	if (result)
		return reinterpret_cast<delegate_generic_class *>(result);
	throw binding_type_exception(typeid(FunctionClass), typeid(object));
}



//**************************************************************************
//  COMMON DELEGATE BASE CLASS
//**************************************************************************

template <class LateBindBase, typename ReturnType, typename... Params>
class delegate_base
{
public:
	// define our traits
	template <class FunctionClass> using traits = delegate_traits<FunctionClass, ReturnType, Params...>;
	using generic_static_func = typename traits<delegate_generic_class>::static_func_type;
	typedef MAME_ABI_CXX_MEMBER_CALL generic_static_func generic_member_func;

	// generic constructor
	delegate_base() = default;

	// copy constructor
	delegate_base(const delegate_base &src)
		: m_function(src.m_function)
		, m_object(src.m_object)
		, m_latebinder(src.m_latebinder)
		, m_raw_function(src.m_raw_function)
		, m_raw_mfp(src.m_raw_mfp)
	{
		if (src.object() && is_mfp())
			m_raw_mfp.update_after_copy(m_function, m_object);
	}

	// copy constructor with late bind
	delegate_base(const delegate_base &src, LateBindBase &object)
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
		: m_latebinder(object)
		, m_raw_mfp(funcptr, object, static_cast<ReturnType *>(nullptr), static_cast<generic_static_func>(nullptr))
	{
		bind(object);
	}

	// construct from const member function with object pointer
	template <class FunctionClass>
	delegate_base(typename traits<FunctionClass>::const_member_func_type funcptr, FunctionClass *object)
		: m_latebinder(object)
		, m_raw_mfp(funcptr, object, static_cast<ReturnType *>(nullptr), static_cast<generic_static_func>(nullptr))
	{
		bind(object);
	}

	// construct from static reference function with object reference
	template <class FunctionClass>
	delegate_base(typename traits<FunctionClass>::static_ref_func_type funcptr, FunctionClass *object)
		: m_function(reinterpret_cast<generic_static_func>(funcptr))
		, m_latebinder(object)
		, m_raw_function(reinterpret_cast<generic_static_func>(funcptr))
	{
		bind(object);
	}

	// copy operator
	delegate_base &operator=(const delegate_base &src)
	{
		if (this != &src)
		{
			m_function = src.m_function;
			m_object = src.m_object;
			m_latebinder = src.m_latebinder;
			m_raw_function = src.m_raw_function;
			m_raw_mfp = src.m_raw_mfp;

			if (src.object() && is_mfp())
				m_raw_mfp.update_after_copy(m_function, m_object);
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
		if ((MAME_DELEGATE_DIFFERENT_MEMBER_ABI) && is_mfp())
			return (*reinterpret_cast<generic_member_func>(m_function))(m_object, std::forward<Params>(args)...);
		else
			return (*m_function)(m_object, std::forward<Params>(args)...);
	}

	// getters
	bool has_object() const { return object() != nullptr; }
	bool isnull() const { return !m_raw_function && m_raw_mfp.isnull(); }
	bool is_mfp() const { return !m_raw_mfp.isnull(); }

	// late binding
	void late_bind(LateBindBase &object)
	{
		if (m_latebinder)
			bind(m_latebinder(object));
	}

protected:
	// return the actual object (not the one we use for calling)
	delegate_generic_class *object() const { return is_mfp() ? m_raw_mfp.real_object(m_object) : m_object; }

	// bind the actual object
	template <typename FunctionClass>
	void bind(FunctionClass *object)
	{
		m_object = reinterpret_cast<delegate_generic_class *>(object);

		// if we're wrapping a member function pointer, handle special stuff
		if (m_object && is_mfp())
			m_raw_mfp.update_after_bind(m_function, m_object);
	}

	// internal state
	generic_static_func                     m_function = nullptr;       // resolved static function pointer
	delegate_generic_class *                m_object = nullptr;         // resolved object to the post-cast object
	delegate_late_bind_helper<LateBindBase> m_latebinder;               // late binding helper
	generic_static_func                     m_raw_function = nullptr;   // raw static function pointer
	delegate_mfp_t<ReturnType>              m_raw_mfp;                  // raw member function pointer
};

} // namespace util::detail



//**************************************************************************
//  NATURAL SYNTAX
//**************************************************************************

// declare the base template
template <typename Signature, class LateBindBase = delegate_late_bind> class delegate;

template <class LateBindBase, typename ReturnType, typename... Params>
class delegate<ReturnType (Params...), LateBindBase> : public util::detail::delegate_base<LateBindBase, ReturnType, Params...>
{
private:
	using basetype = util::detail::delegate_base<LateBindBase, ReturnType, Params...>;
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

	template <typename T> using suitable_functoid = std::is_invocable_r<ReturnType, T, Params...>;

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

	delegate(delegate const &src, LateBindBase &object)
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

#endif // MAME_LIB_UTIL_DELEGATE_H
