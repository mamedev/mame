// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    devdelegate.h

    Delegates that are late-bound to MAME devices.

***************************************************************************/
#ifndef MAME_EMU_DEVDELEGATE_H
#define MAME_EMU_DEVDELEGATE_H

#pragma once

#include "delegate.h"

#include <array>
#include <functional>
#include <type_traits>
#include <utility>


namespace emu {

//**************************************************************************
//  FORWARD DECLARATIONS
//**************************************************************************

template <typename Signature, unsigned Count> class device_delegate_array;



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

namespace detail {

// ======================> device_delegate_helper

// device_delegate_helper does non-template work
class device_delegate_helper
{
public:
	// accessors
	char const *finder_tag() const { return m_tag; }
	std::pair<device_t &, char const *> finder_target() const { return std::make_pair(m_base, m_tag); }

protected:
	// construct/assign
	device_delegate_helper(device_t &owner, char const *tag = nullptr) : m_base(owner), m_tag(tag) { }
	template <class DeviceClass, bool Required> device_delegate_helper(device_finder<DeviceClass, Required> const &finder);
	device_delegate_helper(device_delegate_helper const &) = default;
	device_delegate_helper &operator=(device_delegate_helper const &) = default;

	// internal helpers
	delegate_late_bind &bound_object() const;
	void set_tag(device_t &object) { m_base = object; m_tag = nullptr; }
	void set_tag(device_t &base, char const *tag) { m_base = base; m_tag = tag; }
	void set_tag(char const *tag);
	template <class DeviceClass, bool Required> void set_tag(device_finder<DeviceClass, Required> const &finder);

private:
	std::reference_wrapper<device_t> m_base;
	char const *m_tag;
};

} // namespace detail


// ======================> named_delegate
template <typename Signature>
class named_delegate : public delegate<Signature>
{
private:
	using basetype = delegate<Signature>;

	char const *m_name = nullptr; // name string

protected:
	template <class FunctionClass> using member_func_type = typename basetype::template member_func_type<FunctionClass>;
	template <class FunctionClass> using const_member_func_type = typename basetype::template const_member_func_type<FunctionClass>;
	template <class FunctionClass> using static_ref_func_type = typename basetype::template static_ref_func_type<FunctionClass>;

public:
	// create a standard set of constructors
	named_delegate() = default;
	named_delegate(named_delegate const &) = default;
	named_delegate(named_delegate const &src, delegate_late_bind &object) : basetype(src, object), m_name(src.m_name) { }
	template <class FunctionClass> named_delegate(member_func_type<FunctionClass> funcptr, char const *name, FunctionClass *object) : basetype(funcptr, object), m_name(name) { }
	template <class FunctionClass> named_delegate(const_member_func_type<FunctionClass> funcptr, char const *name, FunctionClass *object) : basetype(funcptr, object), m_name(name) { }
	template <class FunctionClass> named_delegate(static_ref_func_type<FunctionClass> funcptr, char const *name, FunctionClass *object) : basetype(funcptr, object), m_name(name) { }
	template <typename T> named_delegate(T &&funcptr, std::enable_if_t<std::is_constructible<std::function<Signature>, T>::value, char const *> name) : basetype(std::forward<T>(funcptr)), m_name(name) { }

	// allow assignment
	named_delegate &operator=(named_delegate const &src) = default;

	char const *name() const { return m_name; }
};

// ======================> device_delegate

// device_delegate is a delegate that wraps with a device tag and can be easily
// late bound without replicating logic everywhere
template <typename Signature> class device_delegate;

template <typename ReturnType, typename... Params>
class device_delegate<ReturnType (Params...)> : protected named_delegate<ReturnType (Params...)>, public detail::device_delegate_helper
{
private:
	using basetype = named_delegate<ReturnType (Params...)>;

	template <class T, class U> struct is_related_device_implementation
	{ static constexpr bool value = std::is_base_of<T, U>::value && std::is_base_of<device_t, U>::value; };
	template <class T, class U> struct is_related_device_interface
	{ static constexpr bool value = std::is_base_of<T, U>::value && std::is_base_of<device_interface, U>::value && !std::is_base_of<device_t, U>::value; };
	template <class T, class U> struct is_related_device
	{ static constexpr bool value = is_related_device_implementation<T, U>::value || is_related_device_interface<T, U>::value; };

	template <class T> static std::enable_if_t<is_related_device_implementation<T, T>::value, device_t &> get_device(T &object) { return object; }
	template <class T> static std::enable_if_t<is_related_device_interface<T, T>::value, device_t &> get_device(T &object) { return object.device(); }

public:
	template <unsigned Count> using array = device_delegate_array<ReturnType (Params...), Count>;

	template <typename T> struct supports_callback
	{ static constexpr bool value = std::is_constructible<device_delegate, device_t &, char const *, T, char const *>::value; };

	// construct/assign
	explicit device_delegate(device_t &owner) : basetype(), detail::device_delegate_helper(owner) { }
	device_delegate(device_delegate const &) = default;
	device_delegate &operator=(device_delegate const &) = default;

	// construct with prototype and target
	device_delegate(basetype const &proto, device_t &object) : basetype(proto, object), detail::device_delegate_helper(object) { }
	device_delegate(device_delegate const &proto, device_t &object) : basetype(proto, object), detail::device_delegate_helper(object) { }

	// construct with base and tag
	template <class D>
	device_delegate(device_t &base, char const *tag, ReturnType (D::*funcptr)(Params...), char const *name)
		: basetype(funcptr, name, static_cast<D *>(nullptr))
		, detail::device_delegate_helper(base, tag)
	{ }
	template <class D>
	device_delegate(device_t &base, char const *tag, ReturnType (D::*funcptr)(Params...) const, char const *name)
		: basetype(funcptr, name, static_cast<D *>(nullptr))
		, detail::device_delegate_helper(base, tag)
	{ }
	template <class D>
	device_delegate(device_t &base, char const *tag, ReturnType (*funcptr)(D &, Params...), char const *name)
		: basetype(funcptr, name, static_cast<D *>(nullptr))
		, detail::device_delegate_helper(base, tag)
	{ }

	// construct with device finder
	template <class D, bool R, class E>
	device_delegate(device_finder<D, R> const &finder, ReturnType (E::*funcptr)(Params...), char const *name)
		: basetype(funcptr, name, static_cast<E *>(nullptr))
		, detail::device_delegate_helper(finder)
	{ }
	template <class D, bool R, class E>
	device_delegate(device_finder<D, R> const &finder, ReturnType (E::*funcptr)(Params...) const, char const *name)
		: basetype(funcptr, name, static_cast<E *>(nullptr))
		, detail::device_delegate_helper(finder)
	{ }
	template <class D, bool R, class E>
	device_delegate(device_finder<D, R> const &finder, ReturnType (*funcptr)(E &, Params...), char const *name)
		: basetype(funcptr, name, static_cast<E *>(nullptr))
		, detail::device_delegate_helper(finder)
	{ }

	// construct with target object
	template <class T, class D>
	device_delegate(T &object, ReturnType (D::*funcptr)(Params...), std::enable_if_t<is_related_device<D, T>::value, char const *> name)
		: basetype(funcptr, name, static_cast<D *>(&object))
		, detail::device_delegate_helper(get_device(object))
	{ }
	template <class T, class D>
	device_delegate(T &object, ReturnType (D::*funcptr)(Params...) const, std::enable_if_t<is_related_device<D, T>::value, char const *> name)
		: basetype(funcptr, name, static_cast<D *>(&object))
		, detail::device_delegate_helper(get_device(object))
	{ }
	template <class T, class D>
	device_delegate(T &object, ReturnType (*funcptr)(D &, Params...), std::enable_if_t<is_related_device<D, T>::value, char const *> name)
		: basetype(funcptr, name, static_cast<D *>(&object))
		, detail::device_delegate_helper(get_device(object))
	{ }

	// construct with callable object
	template <typename T>
	device_delegate(device_t &owner, T &&funcptr, std::enable_if_t<std::is_constructible<std::function<ReturnType (Params...)>, T>::value, char const *> name)
		: basetype(std::forward<T>(funcptr), name)
		, detail::device_delegate_helper(owner)
	{ basetype::operator=(basetype(std::forward<T>(funcptr), name)); }

	// setters that implicitly bind to the current device
	template <class D> void set(ReturnType (D::*funcptr)(Params...), char const *name)
	{ basetype::operator=(basetype(funcptr, name, static_cast<D *>(nullptr))); set_tag(nullptr); }
	template <class D> void set(ReturnType (D::*funcptr)(Params...) const, char const *name)
	{ basetype::operator=(basetype(funcptr, name, static_cast<D *>(nullptr))); set_tag(nullptr); }
	template <class D> void set(ReturnType (*funcptr)(D &, Params...), char const *name)
	{ basetype::operator=(basetype(funcptr, name, static_cast<D *>(nullptr))); set_tag(nullptr); }

	// setters that take a tag-like object specifying the target
	template <typename T, class D> void set(T &&tag, ReturnType (D::*funcptr)(Params...), char const *name)
	{ basetype::operator=(basetype(funcptr, name, static_cast<D *>(nullptr))); set_tag(std::forward<T>(tag)); }
	template <typename T, class D> void set(T &&tag, ReturnType (D::*funcptr)(Params...) const, char const *name)
	{ basetype::operator=(basetype(funcptr, name, static_cast<D *>(nullptr))); set_tag(std::forward<T>(tag)); }
	template <typename T, class D> void set(T &&tag, ReturnType (*funcptr)(D &, Params...), char const *name)
	{ basetype::operator=(basetype(funcptr, name, static_cast<D *>(nullptr))); set_tag(std::forward<T>(tag)); }

	// setters that take a target object
	template <class T, class D> std::enable_if_t<std::is_base_of<D, T>::value> set(T &object, ReturnType (D::*funcptr)(Params...), char const *name)
	{ basetype::operator=(basetype(funcptr, name, static_cast<D *>(&object))); set_tag(finder_target().first); }
	template <class T, class D> std::enable_if_t<std::is_base_of<D, T>::value> set(T &object, ReturnType (D::*funcptr)(Params...) const, char const *name)
	{ basetype::operator=(basetype(funcptr, name, static_cast<D *>(&object))); set_tag(finder_target().first); }
	template <class T, class D> std::enable_if_t<std::is_base_of<D, T>::value> set(T &object, ReturnType (*funcptr)(D &, Params...), char const *name)
	{ basetype::operator=(basetype(funcptr, name, static_cast<D *>(&object))); set_tag(finder_target().first); }

	// setter that takes a functoid
	template <typename T> std::enable_if_t<std::is_constructible<std::function<ReturnType (Params...)>, T>::value> set(T &&funcptr, char const *name)
	{ basetype::operator=(basetype(std::forward<T>(funcptr), name)); }

	// unsetter
	void set(std::nullptr_t)
	{ basetype::operator=(basetype()); set_tag(finder_target().first); }

	// perform the binding
	void resolve() { if (!basetype::isnull() && !basetype::has_object()) basetype::late_bind(bound_object()); }

	// accessors
	using basetype::operator();
	using basetype::isnull;
	using basetype::has_object;
	using basetype::name;
};


// simplifies constructing an array of delegates with a single owner
template <typename Signature, unsigned Count>
class device_delegate_array : public std::array<device_delegate<Signature>, Count>
{
private:
	template <unsigned... V>
	device_delegate_array(device_t &owner, std::integer_sequence<unsigned, V...> const &)
		: std::array<device_delegate<Signature>, Count>{ make_one<V>(owner)... }
	{
	}

	template <unsigned N> device_delegate<Signature> make_one(device_t &owner)
	{
		return device_delegate<Signature>(owner);
	}

public:
	using std::array<device_delegate<Signature>, Count>::array;

	device_delegate_array(device_t &owner)
		: device_delegate_array(owner, std::make_integer_sequence<unsigned, Count>())
	{
	}

	void resolve_all()
	{
		for (device_delegate<Signature> &elem : *this)
			elem.resolve();
	}
};

} // namespace emu


using emu::named_delegate;
using emu::device_delegate;

#endif // MAME_EMU_DEVDELEGATE_H
