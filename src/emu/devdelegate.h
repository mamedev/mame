/***************************************************************************

    devdelegate.h

    Delegates that are late-bound to MAME devices.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __DEVDELEGATE_H__
#define __DEVDELEGATE_H__

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
	delegate_late_bind &bound_object(device_t &search_root);
	static const char *safe_tag(device_t *object);

	// internal state
	const char *m_device_name;
};


// ======================> device_delegate

// device_delegate is a delegate that wraps with a device tag and can be easily
// late bound without replicating logic everywhere
template<typename _Signature>
class device_delegate : public delegate<_Signature>, device_delegate_helper
{
	typedef device_delegate<_Signature> thistype;
	typedef delegate<_Signature> basetype;

public:
	// provide the same constructors as the base class
	device_delegate() : basetype(), device_delegate_helper(NULL) { }
	device_delegate(const basetype &src) : basetype(src), device_delegate_helper(src.m_device_name) { }
	device_delegate(const basetype &src, delegate_late_bind &object) : basetype(src, object), device_delegate_helper(src.m_device_name) { }
	template<class _FunctionClass> device_delegate(typename basetype::template traits<_FunctionClass>::member_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object), device_delegate_helper(safe_tag(dynamic_cast<device_t *>(object))) { }
	template<class _FunctionClass> device_delegate(typename basetype::template traits<_FunctionClass>::static_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object), device_delegate_helper(safe_tag(dynamic_cast<device_t *>(object))) { }
	template<class _FunctionClass> device_delegate(typename basetype::template traits<_FunctionClass>::static_ref_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object), device_delegate_helper(safe_tag(dynamic_cast<device_t *>(object))) { }
	device_delegate &operator=(const thistype &src) { *static_cast<basetype *>(this) = src; m_device_name = src.m_device_name; return *this; }

	// provide additional constructors that take a device name string
	template<class _FunctionClass> device_delegate(typename basetype::template traits<_FunctionClass>::member_func_type funcptr, const char *name, const char *devname) : basetype(funcptr, name, (_FunctionClass *)0), device_delegate_helper(devname) { }
	template<class _FunctionClass> device_delegate(typename basetype::template traits<_FunctionClass>::member_func_type funcptr, const char *name, const char *devname, _FunctionClass *object) : basetype(funcptr, name, (_FunctionClass *)0), device_delegate_helper(devname) { }
	template<class _FunctionClass> device_delegate(typename basetype::template traits<_FunctionClass>::static_func_type funcptr, const char *name, const char *devname, _FunctionClass *object) : basetype(funcptr, name, (_FunctionClass *)0), device_delegate_helper(devname) { }
	template<class _FunctionClass> device_delegate(typename basetype::template traits<_FunctionClass>::static_ref_func_type funcptr, const char *name, const char *devname, _FunctionClass *object) : basetype(funcptr, name, (_FunctionClass *)0), device_delegate_helper(devname) { }
	device_delegate(typename basetype::template traits<device_t>::static_func_type funcptr, const char *name) : basetype(funcptr, name, (device_t *)0), device_delegate_helper(NULL) { }
	device_delegate(typename basetype::template traits<device_t>::static_ref_func_type funcptr, const char *name) : basetype(funcptr, name, (device_t *)0), device_delegate_helper(NULL) { }

	// and constructors that provide a search root
	device_delegate(const thistype &src, device_t &search_root) : basetype(src), device_delegate_helper(src.m_device_name) { bind_relative_to(search_root); }

	// perform the binding
	void bind_relative_to(device_t &search_root) { if (!basetype::isnull()) basetype::late_bind(bound_object(search_root)); }
};


#endif	/* __DEVDELEGATE_H__ */
