// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    devdelegate.h

    Delegates that are late-bound to MAME devices.

***************************************************************************/

#ifndef MAME_EMU_DEVDELEGATE_H
#define MAME_EMU_DEVDELEGATE_H

#pragma once

#include "delegate.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_delegate_helper

// device_delegate_helper does non-template work
class device_delegate_helper
{
protected:
	// constructor
	device_delegate_helper(const char *devname) : m_device_name(devname) { }

	// internal helpers
	delegate_late_bind &bound_object(device_t &search_root) const;
	static const char *safe_tag(device_t *object);

	// internal state
	const char *m_device_name;
};


// ======================> named_delegate
template<typename _Signature>
class named_delegate : public delegate<_Signature>
{
	typedef delegate<_Signature> basetype;

public:
	// create a standard set of constructors
	named_delegate() : basetype(), m_name(nullptr) { }
	explicit named_delegate(const basetype &src) : basetype(src), m_name(src.m_name) { }
	named_delegate(const basetype &src, delegate_late_bind &object) : basetype(src, object), m_name(src.m_name) { }
	template<class _FunctionClass> named_delegate(typename basetype::template traits<_FunctionClass>::member_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, object), m_name(name) { }
	template<class _FunctionClass> named_delegate(typename basetype::template traits<_FunctionClass>::const_member_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, object), m_name(name) { }
	explicit named_delegate(std::function<_Signature> funcptr, const char *name) : basetype(funcptr), m_name(name) { }
	template<class _FunctionClass> named_delegate(typename basetype::template traits<_FunctionClass>::static_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, object), m_name(name) { }
	template<class _FunctionClass> named_delegate(typename basetype::template traits<_FunctionClass>::static_ref_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, object), m_name(name) { }
	named_delegate &operator=(const basetype &src) { *static_cast<basetype *>(this) = src; m_name = src.m_name; return *this; }

	const char *name() const { return m_name; }
private:
	const char *                m_name;             // name string
};

// ======================> device_delegate

// device_delegate is a delegate that wraps with a device tag and can be easily
// late bound without replicating logic everywhere
template<typename _Signature>
class device_delegate : public named_delegate<_Signature>, device_delegate_helper
{
	typedef device_delegate<_Signature> thistype;
	typedef named_delegate<_Signature> basetype;

public:
	// provide the same constructors as the base class
	device_delegate() : basetype(), device_delegate_helper(nullptr) { }
	device_delegate(const basetype &src) : basetype(src), device_delegate_helper(src.m_device_name) { }
	device_delegate(const basetype &src, delegate_late_bind &object) : basetype(src, object), device_delegate_helper(src.m_device_name) { }
	template<class _FunctionClass> device_delegate(typename basetype::template traits<_FunctionClass>::member_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object), device_delegate_helper(safe_tag(dynamic_cast<device_t *>(object))) { }
	template<class _FunctionClass> device_delegate(typename basetype::template traits<_FunctionClass>::const_member_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object), device_delegate_helper(safe_tag(dynamic_cast<device_t *>(object))) { }
	template<class _FunctionClass> device_delegate(typename basetype::template traits<_FunctionClass>::static_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object), device_delegate_helper(safe_tag(dynamic_cast<device_t *>(object))) { }
	template<class _FunctionClass> device_delegate(typename basetype::template traits<_FunctionClass>::static_ref_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object), device_delegate_helper(safe_tag(dynamic_cast<device_t *>(object))) { }
	device_delegate(std::function<_Signature> funcptr, const char *name) : basetype(funcptr,name), device_delegate_helper(nullptr) { }
	device_delegate &operator=(const thistype &src) { *static_cast<basetype *>(this) = src; m_device_name = src.m_device_name; return *this; }

	// provide additional constructors that take a device name string
	template<class _FunctionClass> device_delegate(typename basetype::template traits<_FunctionClass>::member_func_type funcptr, const char *name, const char *devname) : basetype(funcptr, name, static_cast<_FunctionClass *>(nullptr)), device_delegate_helper(devname) { }
	template<class _FunctionClass> device_delegate(typename basetype::template traits<_FunctionClass>::member_func_type funcptr, const char *name, const char *devname, _FunctionClass *) : basetype(funcptr, name, static_cast<_FunctionClass *>(nullptr)), device_delegate_helper(devname) { }
	template<class _FunctionClass> device_delegate(typename basetype::template traits<_FunctionClass>::const_member_func_type funcptr, const char *name, const char *devname) : basetype(funcptr, name, static_cast<_FunctionClass *>(nullptr)), device_delegate_helper(devname) { }
	template<class _FunctionClass> device_delegate(typename basetype::template traits<_FunctionClass>::const_member_func_type funcptr, const char *name, const char *devname, _FunctionClass *) : basetype(funcptr, name, static_cast<_FunctionClass *>(nullptr)), device_delegate_helper(devname) { }
	template<class _FunctionClass> device_delegate(typename basetype::template traits<_FunctionClass>::static_func_type funcptr, const char *name, const char *devname, _FunctionClass *) : basetype(funcptr, name, static_cast<_FunctionClass *>(nullptr)), device_delegate_helper(devname) { }
	template<class _FunctionClass> device_delegate(typename basetype::template traits<_FunctionClass>::static_ref_func_type funcptr, const char *name, const char *devname, _FunctionClass *) : basetype(funcptr, name, static_cast<_FunctionClass *>(nullptr)), device_delegate_helper(devname) { }
	device_delegate(typename basetype::template traits<device_t>::static_func_type funcptr, const char *name) : basetype(funcptr, name, static_cast<device_t *>(nullptr)), device_delegate_helper(nullptr) { }
	device_delegate(typename basetype::template traits<device_t>::static_ref_func_type funcptr, const char *name) : basetype(funcptr, name, static_cast<device_t *>(nullptr)), device_delegate_helper(nullptr) { }

	// and constructors that provide a search root
	device_delegate(const thistype &src, device_t &search_root) : basetype(src), device_delegate_helper(src.m_device_name) { bind_relative_to(search_root); }

	// perform the binding
	void bind_relative_to(device_t &search_root) { if (!basetype::isnull()) basetype::late_bind(bound_object(search_root)); }

	// getter (for validation purposes)
	const char *device_name() const { return m_device_name; }
};


#endif  /* MAME_EMU_DEVDELEGATE_H */
