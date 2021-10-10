// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/// \file
/// \brief Dynamic derived class creator
///
/// Allows on-the-fly creation of derived classes that override selected
/// virtual member functions of the base class.  The overridden base
/// class member functions can be changed even while instances exist.
/// This is particularly useful in conjunction with code generation.
///
/// Only implemented for configurations where it is possible to define a
/// non-member function that uses the same calling convention as a
/// non-static member function, and subject to a number of limitations
/// on classes that can be used as base classes.
///
/// Itanium C++ ABI support is reasonably complete.
///
/// MSVC C++ABI support is subject to additional limitations:
/// * Instances of dynamic derived classes appear to be instance of
///   their respective base classes when subject to RTTI introspection.
/// * Member functions returning class, structure or union types cannot
///   be overridden.
/// * Member functions returning scalar types that are not returned in
///   registers cannot be overridden.
/// * Member functions cannot be overridden for classes built with clang
///   with optimisation disabled.
/// * Base class implementations of member functions can only be
///   resolved if they are virtual member functions that can be
///   overridden.
/// * Overriding member functions and resolving base class
///   implementations of member functions is substantially more
///   expensive than for the Itanium C++ ABI.
/// * Only x86-64 targets are supported.
///
/// It goes without saying that this is not something you are supposed
/// to do.  This depends heavily on implementation details, and careful
/// use by the developer.  Use it at your own risk.
#ifndef MAME_LIB_UTIL_DYNAMICCLASS_H
#define MAME_LIB_UTIL_DYNAMICCLASS_H

#pragma once

#include "abi.h"

#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <utility>


namespace util {

namespace detail {

/// \brief Dynamic derived class base
///
/// Provides common members for dynamic derived classes that do not
/// depend on the size of the virtual table.
class dynamic_derived_class_base
{
protected:
	/// \brief Size of a member function virtual table entry
	///
	/// The size of an entry representing a virtual member function in a
	/// virtual table, as a multiple of the size of a pointer.
	static constexpr std::size_t MEMBER_FUNCTION_SIZE = MAME_ABI_CXX_VTABLE_FNDESC ? MAME_ABI_FNDESC_SIZE : 1;

	/// \brief Virtual table entries before first member function
	///
	/// The number of pointer-sized entries before the first entry that
	/// is part of a member function representation.
	///
	/// For the MSVC C++ ABI, only one of these is an ABI requirement
	/// (the pointer to the complete object locator).  The other isused
	/// internally to allow the base virtual table pointer to be
	/// recovered.
	static constexpr std::size_t VTABLE_PREFIX_ENTRIES = (MAME_ABI_CXX_TYPE == MAME_ABI_CXX_MSVC) ? (1 + 1) : 2;

	/// \brief Virtual table entries generated for a virtual destructor
	///
	/// The number of member function entries generated in the virtual
	/// table for a virtual destructor.
	static constexpr std::size_t VTABLE_DESTRUCTOR_ENTRIES = (MAME_ABI_CXX_TYPE == MAME_ABI_CXX_MSVC) ? 1 : 2;

	/// \brief Virtual table index for base virtual table recovery
	///
	/// Index of the virtual table entry used for recovering the base
	/// class virtual table pointer, relative to the location an
	/// instance virtual table pointer points to.
	static constexpr std::ptrdiff_t VTABLE_BASE_RECOVERY_INDEX = (MAME_ABI_CXX_TYPE == MAME_ABI_CXX_MSVC) ? -2 : -1;

	/// \brief Single inheritance class type info equivalent structure
	///
	/// Structure equivalent to the implementation of std::type_info for
	/// a class with a single direct base class for the Itanium C++ ABI.
	struct itanium_si_class_type_info_equiv
	{
		void const *vptr;                   ///< Pointer to single inheritance class type info virtual table
		char const *name;                   ///< Mangled name of the class
		std::type_info const *base_type;    ///< Pointer to type info for the base class
	};

	/// \brief Member function pointer equivalent structure
	///
	/// Structure equivalent to the representation of a pointer to a
	/// member function for the Itanium C++ ABI.
	struct itanium_member_function_pointer_equiv
	{
		uintptr_t ptr;      ///< Function pointer or virtual table index
		ptrdiff_t adj;      ///< Offset to add to \c this pointer before call

		constexpr bool is_virtual() const
		{
			return (MAME_ABI_CXX_ITANIUM_MFP_TYPE == MAME_ABI_CXX_ITANIUM_MFP_ARM) ? bool(adj & 1) : bool(ptr & 1);
		}

		constexpr uintptr_t function_pointer() const
		{
			//assert(!is_virtual()); not constexpr
			return ptr;
		}

		constexpr uintptr_t virtual_table_entry_offset() const
		{
			//assert(is_virtual()); not constexpr
			return ptr - ((MAME_ABI_CXX_ITANIUM_MFP_TYPE == MAME_ABI_CXX_ITANIUM_MFP_STANDARD) ? 1 : 0);
		}

		constexpr ptrdiff_t this_pointer_offset() const
		{
			return adj >> ((MAME_ABI_CXX_ITANIUM_MFP_TYPE == MAME_ABI_CXX_ITANIUM_MFP_ARM) ? 1 : 0);
		}
	};

	struct msvc_complete_object_locator_equiv;
	struct msvc_type_info_equiv;

	/// \brief Single inheritance member function pointer equivalent
	///
	/// Structure equivalent to the representation of a pointer to a
	/// member function of a single inheritance class for the MSVC C++
	/// ABI.
	struct msvc_si_member_function_pointer_equiv
	{
		uintptr_t ptr;      ///< Pointer to function, PLT entry, or virtual member call thunk
	};

	/// \brief Multiple inheritance member function pointer equivalent
	///
	/// Structure equivalent to the representation of a pointer to a
	/// member function of a Multiple inheritance class for the MSVC C++
	/// ABI.
	struct msvc_mi_member_function_pointer_equiv : msvc_si_member_function_pointer_equiv
	{
		int adj;            ///< Offset to \c this pointer to add before call
	};

#if MAME_ABI_CXX_TYPE == MAME_ABI_CXX_MSVC
	using member_function_pointer_equiv = msvc_mi_member_function_pointer_equiv;

	template <typename T>
	using supported_return_type = std::bool_constant<std::is_void_v<T> || std::is_scalar_v<T> || std::is_reference_v<T> >;
#else
	using member_function_pointer_equiv = itanium_member_function_pointer_equiv;

	template <typename T>
	using supported_return_type = std::true_type;
#endif

	template <typename T>
	struct member_function_pointer_pun
	{
		static_assert(
#if MAME_ABI_CXX_TYPE == MAME_ABI_CXX_MSVC
				(sizeof(T) == sizeof(msvc_si_member_function_pointer_equiv)) || (sizeof(T) == sizeof(msvc_mi_member_function_pointer_equiv)),
#else
				sizeof(T) == sizeof(itanium_member_function_pointer_equiv),
#endif
				"Unsupported member function pointer representation");

		union type
		{
			T ptr;
			member_function_pointer_equiv equiv;
		};
	};

	template <typename T>
	using member_function_pointer_pun_t = typename member_function_pointer_pun<T>::type;

	template <class Base, typename Extra>
	class value_type
	{
	private:
		template <typename... T, typename... U, std::size_t... N, std::size_t... O>
		value_type(
				std::tuple<T...> &a,
				std::tuple<U...> &b,
				std::integer_sequence<std::size_t, N...>,
				std::integer_sequence<std::size_t, O...>) :
			base(std::forward<T>(std::get<N>(a))...),
			extra(std::forward<U>(std::get<O>(b))...)
		{
		}

	public:
		template <typename... T>
		value_type(T &&... a) :
			base(std::forward<T>(a)...)
		{
		}

		template <typename... T, typename... U>
		value_type(std::piecewise_construct_t, std::tuple<T...> a, std::tuple<U...> b) :
			value_type(a, b, std::make_integer_sequence<std::size_t, sizeof...(T)>(), std::make_integer_sequence<std::size_t, sizeof...(U)>())
		{
		}

		template <typename R, typename... T>
		std::pair<R MAME_ABI_CXX_MEMBER_CALL (*)(void *, T...), void *> resolve_base_member_function(
				R (Base::*func)(T...))
		{
			return dynamic_derived_class_base::resolve_base_member_function(base, func);
		}

		template <typename R, typename... T>
		std::pair<R MAME_ABI_CXX_MEMBER_CALL (*)(void const *, T...), void const *> resolve_base_member_function(
				R (Base::*func)(T...) const) const
		{
			return dynamic_derived_class_base::resolve_base_member_function(base, func);
		}

		template <typename R, typename... T>
		R call_base_member_function(R (Base::*func)(T...), T... args)
		{
			auto const resolved = dynamic_derived_class_base::resolve_base_member_function(base, func);
			return resolved.first(resolved.second, std::forward<T>(args)...);
		}

		template <typename R, typename... T>
		R call_base_member_function(R (Base::*func)(T...) const, T... args) const
		{
			auto const resolved = dynamic_derived_class_base::resolve_base_member_function(base, func);
			return resolved.first(resolved.second, std::forward<T>(args)...);
		}

		Base base;
		Extra extra;
	};

	template <class Base>
	class value_type<Base, void>
	{
	public:
		template <typename... T> value_type(T &&... args) : base(std::forward<T>(args)...) { }

		template <typename R, typename... T>
		std::pair<R MAME_ABI_CXX_MEMBER_CALL (*)(void *, T...), void *> resolve_base_member_function(
				R (Base::*func)(T...))
		{
			return dynamic_derived_class_base::resolve_base_member_function(base, func);
		}

		template <typename R, typename... T>
		std::pair<R MAME_ABI_CXX_MEMBER_CALL (*)(void const *, T...), void const *> resolve_base_member_function(
				R (Base::*func)(T...) const) const
		{
			return dynamic_derived_class_base::resolve_base_member_function(base, func);
		}

		template <typename R, typename... T>
		R call_base_member_function(R (Base::*func)(T...), T... args)
		{
			auto const resolved = dynamic_derived_class_base::resolve_base_member_function(base, func);
			return resolved.first(resolved.second, std::forward<T>(args)...);
		}

		template <typename R, typename... T>
		R call_base_member_function(R (Base::*func)(T...) const, T... args) const
		{
			auto const resolved = dynamic_derived_class_base::resolve_base_member_function(base, func);
			return resolved.first(resolved.second, std::forward<T>(args)...);
		}

		Base base;
	};

	template <class Base, typename Extra, typename Enable = void>
	struct destroyer;

	/// \brief Destroyer for base classes with virtual destructors
	///
	/// Provides implementations of the complete object destructor and
	/// deleting destructor required to allow deleting instances of
	/// dynamic derived classes through pointers to the base class type.
	/// \tparam Base The base class type.
	/// \tparam Extra The extra data type.
	template <class Base, typename Extra>
	struct destroyer<Base, Extra, std::enable_if_t<std::has_virtual_destructor_v<Base> > >
	{
		using pointer_type = std::unique_ptr<Base>;

		static void MAME_ABI_CXX_MEMBER_CALL complete_object_destructor(
				value_type<Base, Extra> &object);

		static void MAME_ABI_CXX_MEMBER_CALL deleting_destructor(
				value_type<Base, Extra> *object);

		static void *MAME_ABI_CXX_MEMBER_CALL scalar_deleting_destructor(
				value_type<Base, Extra> *object,
				unsigned int flags);
	};

	/// \brief Destroyer for base classes without virtual destructors
	///
	/// Used as the deleter type allowing a \c std::unique_ptr to
	/// correctly delete instances of a dynamic derived class when the
	/// the base class type does not have a virtual destructor.
	/// \tparam Base The base class type.
	/// \tparam Extra The extra data type.
	template <class Base, typename Extra>
	struct destroyer<Base, Extra, std::enable_if_t<!std::has_virtual_destructor_v<Base> > >
	{
		static_assert(sizeof(Base) == sizeof(value_type<Base, Extra>), "Value type does not have expected layout");

		using pointer_type = std::unique_ptr<Base, destroyer>;

		void operator()(Base *object) const;
	};

	dynamic_derived_class_base(std::string_view name);
	~dynamic_derived_class_base();

	static std::size_t resolve_virtual_member_slot(member_function_pointer_equiv &slot, std::size_t size);

#if MAME_ABI_CXX_TYPE == MAME_ABI_CXX_MSVC
	msvc_type_info_equiv *m_type_info;
#else
	itanium_si_class_type_info_equiv m_type_info;   ///< Type info for the dynamic derived class
#endif
	std::string m_name;                             ///< Storage for the class name (mangled for Itanium, undecorated for MSVC)
	void const *m_base_vtable;                      ///< Saved base class virtual table pointer

private:
	static std::ptrdiff_t base_vtable_offset();

	template <typename Base>
	static std::uintptr_t const *get_base_vptr(Base const &object);

	template <typename Base>
	static void restore_base_vptr(Base &object);

	template <typename Base, typename R, typename... T>
	static std::pair<R MAME_ABI_CXX_MEMBER_CALL (*)(void *, T...), void *> resolve_base_member_function(
			Base &object,
			R (Base::*func)(T...));

	template <typename Base, typename R, typename... T>
	static std::pair<R MAME_ABI_CXX_MEMBER_CALL (*)(void const *, T...), void const *> resolve_base_member_function(
			Base const &object,
			R (Base::*func)(T...) const);
};

} // namespace detail



/// \brief Dynamic derived class
///
/// Allows dynamically creating classes derived from a supplied base
/// class and overriding virtual member functions.  A class must meet a
/// number of requirements to be suitable for use as a base class.  Most
/// of these requirements cannot be checked automatically.  It is the
/// developer's responsibility to ensure the requirements are met.
///
/// The dynamic derived class object must not be destroyed until after
/// all instances of the class have been destroyed.
///
/// When destroying an instance of the dynamic derived class, the base
/// class vtable is restored before the extra data destructor is called.
/// This allows the extra data type to hold a smart pointer to the
/// dynamic derived class so it can be cleaned up automatically when all
/// instances are destroyed.  However, it means you must keep in mind
/// that the base class implementation of all virtual member functions
/// will be restored before the extra data destructor is called.
/// \tparam Base Base class for the dynamic derived class.  Must meet
///   the following requirements:
///   * Must be a concrete class with at least one public constructor
///     and a public destructor.
///   * Must have at least one virtual member function.
///   * Must not have any direct or indirect virtual base classes.
///   * Must have no secondary virtual tables.  This means the class and
///     all of its direct and indirect base classes may only inherit
///     virtual member functions from one base class at most.
///   * If the class has a virtual destructor, it must be declared
///     declared before any other virtual member functions in the least
///     derived base class containing virtual member functions.
/// \tparam Extra Extra data type, or \c void if not required.  Must be
///   a concrete type with at least one public constructor and a public
///   destructor.
/// \tparam VirtualCount The total number of virtual member functions of
///   the base class, excluding the virtual destructor if present.  This
///   must be correct, and cannot be checked automatically.  It is the
///   developer's responsibility to ensure the value is correct.  This
///   potentially includes additional entries for overridden member
///   functions with covariant return types and implicitly-declared
///   assignment operators.
template <class Base, typename Extra, std::size_t VirtualCount>
class dynamic_derived_class : private detail::dynamic_derived_class_base
{
public:
	/// \brief Type used to store base class and extra data
	///
	/// Has a member \c base of the base class type, and a member
	/// \c extra of the extra data type if it is not \c void.
	///
	/// Provides \c resolve_base_member_function and
	/// \c call_base_member_function member functions to assist with
	/// calling the base implementation of overridden virtual member
	/// functions.
	using type = value_type<Base, Extra>;

	/// \brief Smart pointer to instance
	///
	/// A unique pointer type suitable for taking ownership of an
	/// instance of the dynamic derived class.
	using pointer = typename destroyer<Base, Extra>::pointer_type;

	dynamic_derived_class(dynamic_derived_class const &) = delete;
	dynamic_derived_class &operator=(dynamic_derived_class const &) = delete;

	dynamic_derived_class(std::string_view name);
	dynamic_derived_class(dynamic_derived_class const &prototype, std::string_view name);

	/// \brief Get type info for dynamic derived class
	///
	/// Gets a reference to the type info for the dynamic derived class.
	/// The reference remains valid and unique as long as the dynamic
	/// derived class is not destroyed.
	/// \return A reference to an object equivalent to \c std::type_info.
	std::type_info const &type_info() const
	{
#if MAME_ABI_CXX_TYPE == MAME_ABI_CXX_MSVC
		return *reinterpret_cast<std::type_info const *>(m_type_info);
#else
		return *reinterpret_cast<std::type_info const *>(&m_type_info);
#endif
	}

	template <typename R, typename... T>
	void override_member_function(R (Base::*slot)(T...), R MAME_ABI_CXX_MEMBER_CALL (*func)(type &, T...));

	template <typename R, typename... T>
	void override_member_function(R (Base::*slot)(T...) const, R MAME_ABI_CXX_MEMBER_CALL (*func)(type const &, T...));

	template <typename R, typename... T>
	void restore_base_member_function(R (Base::*slot)(T...));

	template <typename... T>
	pointer instantiate(type *&object, T &&... args);

private:
	static_assert(sizeof(std::uintptr_t) == sizeof(std::ptrdiff_t), "Pointer and pointer difference must be the same size");
	static_assert(sizeof(void *) == sizeof(void (*)()), "Code and data pointers must be the same size");
	static_assert(std::is_polymorphic_v<Base>, "Base class must be polymorphic");

	static constexpr std::size_t FIRST_OVERRIDABLE_MEMBER_OFFSET = std::has_virtual_destructor_v<Base> ? VTABLE_DESTRUCTOR_ENTRIES : 0;
	static constexpr std::size_t VIRTUAL_MEMBER_FUNCTION_COUNT = VirtualCount + FIRST_OVERRIDABLE_MEMBER_OFFSET;
	static constexpr std::size_t VTABLE_SIZE = VTABLE_PREFIX_ENTRIES + (VIRTUAL_MEMBER_FUNCTION_COUNT * MEMBER_FUNCTION_SIZE);

	void override_member_function(member_function_pointer_equiv &slot, std::uintptr_t func, std::size_t size);

	std::array<std::uintptr_t, VTABLE_SIZE> m_vtable;
	std::bitset<VirtualCount> m_overridden;
};

} // namespace util

#endif // MAME_LIB_UTIL_DYNAMICCLASS_H
