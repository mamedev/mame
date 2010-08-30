/***************************************************************************

    delegate.h

    Templates and classes to enable delegates for callbacks.

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

#pragma once

#ifndef __DELEGATE_H__
#define __DELEGATE_H__


//**************************************************************************
//  MACROS
//**************************************************************************

// types of delegates supported
#define DELEGATE_TYPE_COMPATIBLE 0
#define DELEGATE_TYPE_INTERNAL 1

// select which one we will be using
#if defined(__GNUC__)
#define USE_DELEGATE_TYPE DELEGATE_TYPE_INTERNAL
#else
#define USE_DELEGATE_TYPE DELEGATE_TYPE_COMPATIBLE
#endif

// nicer macros to hide the template gobblety-gook and to pass the names
#define create_member_name(_class, _member, _name)	_create_member<_class, &_class::_member>(_name)
#define create_member(_class, _member)				_create_member<_class, &_class::_member>(#_class "::" #_member)
#define create_static(_class, _func)				_crate_static<_class, &_func>(#_func)



//**************************************************************************
//  HELPER CLASSES
//**************************************************************************

// ======================> generic_class

// define a dummy generic class that is just straight single-inheritance
#ifdef _MSC_VER
class __single_inheritance generic_class;
class delegate_generic_class { };
#else
class delegate_generic_class;
#endif


// ======================> bindable_object

// define a bindable_object base class that must be at the root of any object
// hierarchy which intends to do late binding
class bindable_object
{
public:
	// virtual destructor to ensure this is a polymorphic class
	bindable_object();
	virtual ~bindable_object();
};

// define a deferred cast helper function that does a proper dynamic_cast
// from a bindable_object to the target class, and returns a delegate_generic_class
template<class _TargetClass>
static delegate_generic_class *deferred_cast(bindable_object &object)
{
	return reinterpret_cast<delegate_generic_class *>(dynamic_cast<_TargetClass *>(&object));
}

// we store pointers to these deferred casting helpers, so make a friendly type for it
typedef delegate_generic_class *(*deferred_cast_func)(bindable_object &object);


// ======================> delegate_base

// simple base class for all delegates to derive from; it is explicitly
// polymorphic so that a delegate_base * can be used as a lowest-common
// denominator, and then downcast to the appropriate type with proper
// type checking
class delegate_base
{
public:
	delegate_base(deferred_cast_func caster, const char *name)
		: m_caster(caster),
		  m_name(name) { }

	virtual ~delegate_base() { }

	// getters
	bool isnull() const { return (m_caster == NULL); }
	bool valid_target(bindable_object &object) const { return ((*m_caster)(object) != NULL); }
	const char *name() const { return m_name; }

protected:
	deferred_cast_func		m_caster;			// pointer to helper function that does the cast
	const char *			m_name;				// name string
};



//**************************************************************************
//  COMPATIBLE DELEGATES
//**************************************************************************

#if (USE_DELEGATE_TYPE == DELEGATE_TYPE_COMPATIBLE)

// ======================> proto_delegate_0param

template<typename _ReturnType>
class proto_delegate_0param : public delegate_base
{
protected:
	// pointer to a static version of the function which takes the object as a first parameter
	typedef _ReturnType (*static_func)(delegate_generic_class *);

public:
	// constructors
	proto_delegate_0param(static_func function = NULL, deferred_cast_func caster = NULL, const char *name = NULL)
		: delegate_base(caster, name),
		  m_function(function) { }

	proto_delegate_0param(const proto_delegate_0param &proto)
		: delegate_base(proto.m_caster, proto.m_name),
		  m_function(proto.m_function) { }

	// create a member function proto-delegate
	template<class _FunctionClass, _ReturnType (_FunctionClass::*_FunctionPtr)()>
	static proto_delegate_0param _create_member(const char *name = NULL)
	{
		return proto_delegate_0param(&method_stub<_FunctionClass, _FunctionPtr>, &deferred_cast<_FunctionClass>, name);
	}

	// create a static function proto-delegate
	template<class _FunctionClass, _ReturnType (*_FunctionPtr)(_FunctionClass *)>
	static proto_delegate_0param _create_static(const char *name = NULL)
	{
		return proto_delegate_0param(reinterpret_cast<static_func>(_FunctionPtr), &deferred_cast<_FunctionClass>, name);
	}

	// equality testing
	bool operator==(const proto_delegate_0param &rhs) const
	{
		return (m_function == rhs.m_function && m_caster == rhs.m_caster);
	}

protected:
	// helper stub that calls the member function
	template<class _FunctionClass, _ReturnType (_FunctionClass::*_FunctionPtr)()>
    static _ReturnType method_stub(delegate_generic_class *object_ptr)
    {
        _FunctionClass *p = reinterpret_cast<_FunctionClass *>(object_ptr);
        return (p->*_FunctionPtr)();
    }

	// internal state
	static_func 			m_function;			// pointer to the stub or static function
};


// ======================> delegate_0param

template<typename _ReturnType>
class delegate_0param : public proto_delegate_0param<_ReturnType>
{
	typedef proto_delegate_0param<_ReturnType> proto_base;

	using delegate_base::m_caster;
	using proto_base::m_function;

public:
	// constructors
	delegate_0param()
		: m_object(NULL) { }

	delegate_0param(proto_base proto)
		: proto_delegate_0param<_ReturnType>(proto),
		  m_object(NULL) { }

	delegate_0param(proto_base proto, bindable_object &object)
		: proto_delegate_0param<_ReturnType>(proto),
		  m_object((*m_caster)(object)) { }

	// bind the actual object
	void bind(bindable_object &object) { m_object = (*m_caster)(object); }

	// call the function
	_ReturnType operator()() const { return (*m_function)(m_object); }

	// testing
	bool has_object() const { return (m_object != NULL); }
	bool operator==(const delegate_0param &rhs) const { return (m_object == rhs.m_object && proto_base::operator==(rhs)); }

protected:
	// internal state
	delegate_generic_class *m_object;			// pointer to the post-cast object
};


// ======================> proto_delegate_1param

template<typename _ReturnType, typename _P1Type>
class proto_delegate_1param : public delegate_base
{
protected:
	// pointer to a static version of the function which takes the object as a first parameter
	typedef _ReturnType (*static_func)(delegate_generic_class *, _P1Type);

public:
	// constructors
	proto_delegate_1param(static_func function = NULL, deferred_cast_func caster = NULL, const char *name = NULL)
		: delegate_base(caster, name),
		  m_function(function) { }

	proto_delegate_1param(const proto_delegate_1param &proto)
		: delegate_base(proto.m_caster, proto.m_name),
		  m_function(proto.m_function) { }

	// create a member function proto-delegate
	template<class _FunctionClass, _ReturnType (_FunctionClass::*_FunctionPtr)(_P1Type)>
	static proto_delegate_1param _create_member(const char *name = NULL)
	{
		return proto_delegate_1param(&method_stub<_FunctionClass, _FunctionPtr>, &deferred_cast<_FunctionClass>, name);
	}

	// create a static function proto-delegate
	template<class _FunctionClass, _ReturnType (*_FunctionPtr)(_FunctionClass *, _P1Type)>
	static proto_delegate_1param _create_static(const char *name = NULL)
	{
		return proto_delegate_1param(reinterpret_cast<static_func>(_FunctionPtr), &deferred_cast<_FunctionClass>, name);
	}

	// equality testing
	bool operator==(const proto_delegate_1param &rhs) const
	{
		return (m_function == rhs.m_function && m_caster == rhs.m_caster);
	}

protected:
	// helper stub that calls the member function
	template<class _FunctionClass, _ReturnType (_FunctionClass::*_FunctionPtr)(_P1Type)>
    static _ReturnType method_stub(delegate_generic_class *object_ptr, _P1Type p1)
    {
        _FunctionClass *p = reinterpret_cast<_FunctionClass *>(object_ptr);
        return (p->*_FunctionPtr)(p1);
    }

	// internal state
	static_func 			m_function;			// pointer to the stub or static function
};


// ======================> delegate_1param

template<typename _ReturnType, typename _P1Type>
class delegate_1param : public proto_delegate_1param<_ReturnType, _P1Type>
{
	typedef proto_delegate_1param<_ReturnType, _P1Type> proto_base;

	using delegate_base::m_caster;
	using proto_base::m_function;

public:
	// constructors
	delegate_1param()
		: m_object(NULL) { }

	delegate_1param(proto_delegate_1param<_ReturnType, _P1Type> proto)
		: proto_delegate_1param<_ReturnType, _P1Type>(proto),
		  m_object(NULL) { }

	delegate_1param(proto_delegate_1param<_ReturnType, _P1Type> proto, bindable_object &object)
		: proto_delegate_1param<_ReturnType, _P1Type>(proto),
		  m_object((*m_caster)(object)) { }

	// bind the actual object
	void bind(bindable_object &object) { m_object = (*m_caster)(object); }

	// call the function
	_ReturnType operator()(_P1Type p1) const { return (*m_function)(m_object, p1); }

	// testing
	bool has_object() const { return (m_object != NULL); }
	bool operator==(const delegate_1param &rhs) const { return (m_object == rhs.m_object && proto_base::operator==(rhs)); }

protected:
	// internal state
	delegate_generic_class *m_object;			// pointer to the post-cast object
};


// ======================> proto_delegate_2param

template<typename _ReturnType, typename _P1Type, typename _P2Type>
class proto_delegate_2param : public delegate_base
{
protected:
	// pointer to a static version of the function which takes the object as a first parameter
	typedef _ReturnType (*static_func)(delegate_generic_class *, _P1Type, _P2Type);

public:
	// constructors
	proto_delegate_2param(static_func function = NULL, deferred_cast_func caster = NULL, const char *name = NULL)
		: delegate_base(caster, name),
		  m_function(function) { }

	proto_delegate_2param(const proto_delegate_2param &proto)
		: delegate_base(proto.m_caster, proto.m_name),
		  m_function(proto.m_function) { }

	// create a member function proto-delegate
	template<class _FunctionClass, _ReturnType (_FunctionClass::*_FunctionPtr)(_P1Type, _P2Type)>
	static proto_delegate_2param _create_member(const char *name = NULL)
	{
		return proto_delegate_2param(&method_stub<_FunctionClass, _FunctionPtr>, &deferred_cast<_FunctionClass>, name);
	}

	// create a static function proto-delegate
	template<class _FunctionClass, _ReturnType (*_FunctionPtr)(_FunctionClass *, _P1Type, _P2Type)>
	static proto_delegate_2param _create_static(const char *name = NULL)
	{
		return proto_delegate_2param(reinterpret_cast<static_func>(_FunctionPtr), &deferred_cast<_FunctionClass>, name);
	}

	// equality testing
	bool operator==(const proto_delegate_2param &rhs) const
	{
		return (m_function == rhs.m_function && m_caster == rhs.m_caster);
	}

protected:
	// helper stub that calls the member function
	template<class _FunctionClass, _ReturnType (_FunctionClass::*_FunctionPtr)(_P1Type, _P2Type)>
    static _ReturnType method_stub(delegate_generic_class *object_ptr, _P1Type p1, _P2Type p2)
    {
        _FunctionClass *p = reinterpret_cast<_FunctionClass *>(object_ptr);
        return (p->*_FunctionPtr)(p1, p2);
    }

	// internal state
	static_func 			m_function;			// pointer to the stub or static function
};


// ======================> delegate_2param

template<typename _ReturnType, typename _P1Type, typename _P2Type>
class delegate_2param : public proto_delegate_2param<_ReturnType, _P1Type, _P2Type>
{
	typedef proto_delegate_2param<_ReturnType, _P1Type, _P2Type> proto_base;

	using delegate_base::m_caster;
	using proto_base::m_function;

public:
	// constructors
	delegate_2param()
		: m_object(NULL) { }

	delegate_2param(proto_delegate_2param<_ReturnType, _P1Type, _P2Type> proto)
		: proto_delegate_2param<_ReturnType, _P1Type, _P2Type>(proto),
		  m_object(NULL) { }

	delegate_2param(proto_delegate_2param<_ReturnType, _P1Type, _P2Type> proto, bindable_object &object)
		: proto_delegate_2param<_ReturnType, _P1Type, _P2Type>(proto),
		  m_object((*m_caster)(object)) { }

	// bind the actual object
	void bind(bindable_object &object) { m_object = (*m_caster)(object); }

	// call the function
	_ReturnType operator()(_P1Type p1, _P2Type p2) const { return (*m_function)(m_object, p1, p2); }

	// testing
	bool has_object() const { return (m_object != NULL); }
	bool operator==(const delegate_2param &rhs) const { return (m_object == rhs.m_object && proto_base::operator==(rhs)); }

protected:
	// internal state
	delegate_generic_class *m_object;			// pointer to the post-cast object
};


// ======================> proto_delegate_3param

template<typename _ReturnType, typename _P1Type, typename _P2Type, typename _P3Type>
class proto_delegate_3param : public delegate_base
{
protected:
	// pointer to a static version of the function which takes the object as a first parameter
	typedef _ReturnType (*static_func)(delegate_generic_class *, _P1Type, _P2Type, _P3Type);

public:
	// constructors
	proto_delegate_3param(static_func function = NULL, deferred_cast_func caster = NULL, const char *name = NULL)
		: delegate_base(caster, name),
		  m_function(function) { }

	proto_delegate_3param(const proto_delegate_3param &proto)
		: delegate_base(proto.m_caster, proto.m_name),
		  m_function(proto.m_function) { }

	// create a member function proto-delegate
	template<class _FunctionClass, _ReturnType (_FunctionClass::*_FunctionPtr)(_P1Type, _P2Type, _P3Type)>
	static proto_delegate_3param _create_member(const char *name = NULL)
	{
		return proto_delegate_3param(&method_stub<_FunctionClass, _FunctionPtr>, &deferred_cast<_FunctionClass>, name);
	}

	// create a static function proto-delegate
	template<class _FunctionClass, _ReturnType (*_FunctionPtr)(_FunctionClass *, _P1Type, _P2Type, _P3Type)>
	static proto_delegate_3param _create_static(const char *name = NULL)
	{
		return proto_delegate_3param(reinterpret_cast<static_func>(_FunctionPtr), &deferred_cast<_FunctionClass>, name);
	}

	// equality testing
	bool operator==(const proto_delegate_3param &rhs) const
	{
		return (m_function == rhs.m_function && m_caster == rhs.m_caster);
	}

protected:
	// helper stub that calls the member function
	template<class _FunctionClass, _ReturnType (_FunctionClass::*_FunctionPtr)(_P1Type, _P2Type, _P3Type)>
    static _ReturnType method_stub(delegate_generic_class *object_ptr, _P1Type p1, _P2Type p2, _P3Type p3)
    {
        _FunctionClass *p = reinterpret_cast<_FunctionClass *>(object_ptr);
        return (p->*_FunctionPtr)(p1, p2, p3);
    }

	// internal state
	static_func 			m_function;			// pointer to the stub or static function
};


// ======================> delegate_3param

template<typename _ReturnType, typename _P1Type, typename _P2Type, typename _P3Type>
class delegate_3param : public proto_delegate_3param<_ReturnType, _P1Type, _P2Type, _P3Type>
{
	typedef proto_delegate_3param<_ReturnType, _P1Type, _P2Type, _P3Type> proto_base;

	using delegate_base::m_caster;
	using proto_base::m_function;

public:
	// constructors
	delegate_3param()
		: m_object(NULL) { }

	delegate_3param(proto_delegate_3param<_ReturnType, _P1Type, _P2Type, _P3Type> proto)
		: proto_delegate_3param<_ReturnType, _P1Type, _P2Type, _P3Type>(proto),
		  m_object(NULL) { }

	delegate_3param(proto_delegate_3param<_ReturnType, _P1Type, _P2Type, _P3Type> proto, bindable_object &object)
		: proto_delegate_3param<_ReturnType, _P1Type, _P2Type, _P3Type>(proto),
		  m_object((*m_caster)(object)) { }

	// bind the actual object
	void bind(bindable_object &object) { m_object = (*m_caster)(object); }

	// call the function
	_ReturnType operator()(_P1Type p1, _P2Type p2, _P3Type p3) const { return (*m_function)(m_object, p1, p2, p3); }

	// testing
	bool has_object() const { return (m_object != NULL); }
	bool operator==(const delegate_3param &rhs) const { return (m_object == rhs.m_object && proto_base::operator==(rhs)); }

protected:
	// internal state
	delegate_generic_class *m_object;			// pointer to the post-cast object
};


// ======================> proto_delegate_4param

template<typename _ReturnType, typename _P1Type, typename _P2Type, typename _P3Type, typename _P4Type>
class proto_delegate_4param : public delegate_base
{
protected:
	// pointer to a static version of the function which takes the object as a first parameter
	typedef _ReturnType (*static_func)(delegate_generic_class *, _P1Type, _P2Type, _P3Type, _P4Type);

public:
	// constructors
	proto_delegate_4param(static_func function = NULL, deferred_cast_func caster = NULL, const char *name = NULL)
		: delegate_base(caster, name),
		  m_function(function) { }

	proto_delegate_4param(const proto_delegate_4param &proto)
		: delegate_base(proto.m_caster, proto.m_name),
		  m_function(proto.m_function) { }

	// create a member function proto-delegate
	template<class _FunctionClass, _ReturnType (_FunctionClass::*_FunctionPtr)(_P1Type, _P2Type, _P3Type, _P4Type)>
	static proto_delegate_4param _create_member(const char *name = NULL)
	{
		return proto_delegate_4param(&method_stub<_FunctionClass, _FunctionPtr>, &deferred_cast<_FunctionClass>, name);
	}

	// create a static function proto-delegate
	template<class _FunctionClass, _ReturnType (*_FunctionPtr)(_FunctionClass *, _P1Type, _P2Type, _P3Type, _P4Type)>
	static proto_delegate_4param _create_static(const char *name = NULL)
	{
		return proto_delegate_4param(reinterpret_cast<static_func>(_FunctionPtr), &deferred_cast<_FunctionClass>, name);
	}

	// equality testing
	bool operator==(const proto_delegate_4param &rhs) const
	{
		return (m_function == rhs.m_function && m_caster == rhs.m_caster);
	}

protected:
	// helper stub that calls the member function
	template<class _FunctionClass, _ReturnType (_FunctionClass::*_FunctionPtr)(_P1Type, _P2Type, _P3Type, _P4Type)>
    static _ReturnType method_stub(delegate_generic_class *object_ptr, _P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4)
    {
        _FunctionClass *p = reinterpret_cast<_FunctionClass *>(object_ptr);
        return (p->*_FunctionPtr)(p1, p2, p3, p4);
    }

	// internal state
	static_func 			m_function;			// pointer to the stub or static function
};


// ======================> delegate_4param

template<typename _ReturnType, typename _P1Type, typename _P2Type, typename _P3Type, typename _P4Type>
class delegate_4param : public proto_delegate_4param<_ReturnType, _P1Type, _P2Type, _P3Type, _P4Type>
{
	typedef proto_delegate_4param<_ReturnType, _P1Type, _P2Type, _P3Type, _P4Type> proto_base;

	using delegate_base::m_caster;
	using proto_base::m_function;

public:
	// constructors
	delegate_4param()
		: m_object(NULL) { }

	delegate_4param(proto_delegate_4param<_ReturnType, _P1Type, _P2Type, _P3Type, _P4Type> proto)
		: proto_delegate_4param<_ReturnType, _P1Type, _P2Type, _P3Type, _P4Type>(proto),
		  m_object(NULL) { }

	delegate_4param(proto_delegate_4param<_ReturnType, _P1Type, _P2Type, _P3Type, _P4Type> proto, bindable_object &object)
		: proto_delegate_4param<_ReturnType, _P1Type, _P2Type, _P3Type, _P4Type>(proto),
		  m_object((*m_caster)(object)) { }

	// bind the actual object
	void bind(bindable_object &object) { m_object = (*m_caster)(object); }

	// call the function
	_ReturnType operator()(_P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4) const { return (*m_function)(m_object, p1, p2, p3, p4); }

	// testing
	bool has_object() const { return (m_object != NULL); }
	bool operator==(const delegate_4param &rhs) const { return (m_object == rhs.m_object && proto_base::operator==(rhs)); }

protected:
	// internal state
	delegate_generic_class *m_object;			// pointer to the post-cast object
};


#endif


//**************************************************************************
//  GCC DELEGATES
//**************************************************************************

#if (USE_DELEGATE_TYPE == DELEGATE_TYPE_INTERNAL)

// a generic function pointer type and a generic member function pointer type
typedef void (*delegate_generic_function)();

// struct describing the contents of a member function pointer
struct delegate_gcc_mfp_internal
{
	union										// first item can be one of two things:
	{
		delegate_generic_function funcptr;		// if even, it's a pointer to the function
		FPTR				vtable_index;		// if odd, it's the byte offset into the vtable
	} u;
	int 					this_delta;			// delta to apply to the 'this' pointer
};

// helper function
delegate_generic_function delegate_convert_raw(delegate_generic_class *&object, delegate_gcc_mfp_internal &mfp);

// global dummy extern
extern delegate_gcc_mfp_internal delegate_gcc_mfp_null;


// ======================> proto_delegate_0param

template<typename _ReturnType>
class proto_delegate_0param : public delegate_base
{
protected:
	// pointer to a static function which takes the object as a first parameter
	typedef _ReturnType (*static_func)(delegate_generic_class *);

public:
	// constructors
	proto_delegate_0param(delegate_gcc_mfp_internal &mfp = delegate_gcc_mfp_null, static_func function = NULL, deferred_cast_func caster = NULL, const char *name = NULL)
		: delegate_base(caster, name),
		  m_function(function),
		  m_rawfunction(mfp) { }

	proto_delegate_0param(const proto_delegate_0param &proto)
		: delegate_base(proto.m_caster, proto.m_name),
		  m_function(proto.m_function),
		  m_rawfunction(proto.m_rawfunction) { }

	// create a member function proto-delegate
	template<class _FunctionClass, _ReturnType (_FunctionClass::*_FunctionPtr)()>
	static proto_delegate_0param _create_member(const char *name = NULL)
	{
		union
		{
			_ReturnType (_FunctionClass::*mfp)();
			delegate_gcc_mfp_internal	internal;
		} tempunion;
		tempunion.mfp = _FunctionPtr;
		return proto_delegate_0param(tempunion.internal, NULL, &deferred_cast<_FunctionClass>, name);
	}

	// create a static function proto-delegate
	template<class _FunctionClass, _ReturnType (*_FunctionPtr)(_FunctionClass *)>
	static proto_delegate_0param _create_static(const char *name = NULL)
	{
		return proto_delegate_0param(delegate_gcc_mfp_null, reinterpret_cast<static_func>(_FunctionPtr), &deferred_cast<_FunctionClass>, name);
	}

	// equality testing
	bool operator==(const proto_delegate_0param &rhs) const
	{
		return (m_function == rhs.m_function && m_caster == rhs.m_caster &&
				m_rawfunction.u.funcptr == rhs.m_rawfunction.u.funcptr &&
				m_rawfunction.this_delta == rhs.m_rawfunction.this_delta);
	}

protected:
	// internal state
	static_func					m_function;			// generic static function pointer
	delegate_gcc_mfp_internal	m_rawfunction;		// raw member function definition
};


// ======================> delegate_0param

template<typename _ReturnType>
class delegate_0param : public proto_delegate_0param<_ReturnType>
{
	typedef _ReturnType (*static_func)(delegate_generic_class *);
	typedef proto_delegate_0param<_ReturnType> proto_base;

	using delegate_base::m_caster;
	using proto_base::m_function;
	using proto_base::m_rawfunction;

public:
	// constructors
	delegate_0param()
		: m_object(NULL) { }

	delegate_0param(proto_base proto)
		: proto_delegate_0param<_ReturnType>(proto),
		  m_object(NULL) { }

	delegate_0param(proto_base proto, bindable_object &object)
		: proto_delegate_0param<_ReturnType>(proto),
		  m_object(NULL) { bind(object); }

	// bind the actual object
	void bind(bindable_object &object)
	{
		m_object = (*m_caster)(object);
		if (m_rawfunction.u.funcptr != NULL)
			m_function = reinterpret_cast<static_func>(delegate_convert_raw(m_object, m_rawfunction));
	}

	// call the function
	_ReturnType operator()() const { return (*m_function)(m_object); }

	// testing
	bool has_object() const { return (m_object != NULL); }
	bool operator==(const delegate_0param &rhs) const { return (m_object == rhs.m_object && proto_base::operator==(rhs)); }

protected:
	// internal state
	delegate_generic_class *m_object;			// pointer to the post-cast object
};


// ======================> proto_delegate_1param

template<typename _ReturnType, typename _P1Type>
class proto_delegate_1param : public delegate_base
{
protected:
	// pointer to a static function which takes the object as a first parameter
	typedef _ReturnType (*static_func)(delegate_generic_class *, _P1Type);

public:
	// constructors
	proto_delegate_1param(delegate_gcc_mfp_internal &mfp = delegate_gcc_mfp_null, static_func function = NULL, deferred_cast_func caster = NULL, const char *name = NULL)
		: delegate_base(caster, name),
		  m_function(function),
		  m_rawfunction(mfp) { }

	proto_delegate_1param(const proto_delegate_1param &proto)
		: delegate_base(proto.m_caster, proto.m_name),
		  m_function(proto.m_function),
		  m_rawfunction(proto.m_rawfunction) { }

	// create a member function proto-delegate
	template<class _FunctionClass, _ReturnType (_FunctionClass::*_FunctionPtr)(_P1Type)>
	static proto_delegate_1param _create_member(const char *name = NULL)
	{
		union
		{
			_ReturnType (_FunctionClass::*mfp)(_P1Type);
			delegate_gcc_mfp_internal	internal;
		} tempunion;
		tempunion.mfp = _FunctionPtr;
		return proto_delegate_1param(tempunion.internal, NULL, &deferred_cast<_FunctionClass>, name);
	}

	// create a static function proto-delegate
	template<class _FunctionClass, _ReturnType (*_FunctionPtr)(_FunctionClass *, _P1Type)>
	static proto_delegate_1param _create_static(const char *name = NULL)
	{
		return proto_delegate_1param(delegate_gcc_mfp_null, reinterpret_cast<static_func>(_FunctionPtr), &deferred_cast<_FunctionClass>, name);
	}

	// equality testing
	bool operator==(const proto_delegate_1param &rhs) const
	{
		return (m_function == rhs.m_function && m_caster == rhs.m_caster &&
				m_rawfunction.u.funcptr == rhs.m_rawfunction.u.funcptr &&
				m_rawfunction.this_delta == rhs.m_rawfunction.this_delta);
	}

protected:
	// internal state
	static_func					m_function;			// generic static function pointer
	delegate_gcc_mfp_internal	m_rawfunction;		// raw member function definition
};


// ======================> delegate_1param

template<typename _ReturnType, typename _P1Type>
class delegate_1param : public proto_delegate_1param<_ReturnType, _P1Type>
{
	typedef _ReturnType (*static_func)(delegate_generic_class *, _P1Type);
	typedef proto_delegate_1param<_ReturnType, _P1Type> proto_base;

	using delegate_base::m_caster;
	using proto_base::m_function;
	using proto_base::m_rawfunction;

public:
	// constructors
	delegate_1param()
		: m_object(NULL) { }

	delegate_1param(proto_base proto)
		: proto_delegate_1param<_ReturnType, _P1Type>(proto),
		  m_object(NULL) { }

	delegate_1param(proto_base proto, bindable_object &object)
		: proto_delegate_1param<_ReturnType, _P1Type>(proto),
		  m_object(NULL) { bind(object); }

	// bind the actual object
	void bind(bindable_object &object)
	{
		m_object = (*m_caster)(object);
		if (m_rawfunction.u.funcptr != NULL)
			m_function = reinterpret_cast<static_func>(delegate_convert_raw(m_object, m_rawfunction));
	}

	// call the function
	_ReturnType operator()(_P1Type p1) const { return (*m_function)(m_object, p1); }

	// testing
	bool has_object() const { return (m_object != NULL); }
	bool operator==(const delegate_1param &rhs) const { return (m_object == rhs.m_object && proto_base::operator==(rhs)); }

protected:
	// internal state
	delegate_generic_class *m_object;			// pointer to the post-cast object
};


// ======================> proto_delegate_2param

template<typename _ReturnType, typename _P1Type, typename _P2Type>
class proto_delegate_2param : public delegate_base
{
protected:
	// pointer to a static function which takes the object as a first parameter
	typedef _ReturnType (*static_func)(delegate_generic_class *, _P1Type, _P2Type);

public:
	// constructors
	proto_delegate_2param(delegate_gcc_mfp_internal &mfp = delegate_gcc_mfp_null, static_func function = NULL, deferred_cast_func caster = NULL, const char *name = NULL)
		: delegate_base(caster, name),
		  m_function(function),
		  m_rawfunction(mfp) { }

	proto_delegate_2param(const proto_delegate_2param &proto)
		: delegate_base(proto.m_caster, proto.m_name),
		  m_function(proto.m_function),
		  m_rawfunction(proto.m_rawfunction) { }

	// create a member function proto-delegate
	template<class _FunctionClass, _ReturnType (_FunctionClass::*_FunctionPtr)(_P1Type, _P2Type)>
	static proto_delegate_2param _create_member(const char *name = NULL)
	{
		union
		{
			_ReturnType (_FunctionClass::*mfp)(_P1Type, _P2Type);
			delegate_gcc_mfp_internal	internal;
		} tempunion;
		tempunion.mfp = _FunctionPtr;
		return proto_delegate_2param(tempunion.internal, NULL, &deferred_cast<_FunctionClass>, name);
	}

	// create a static function proto-delegate
	template<class _FunctionClass, _ReturnType (*_FunctionPtr)(_FunctionClass *, _P1Type, _P2Type)>
	static proto_delegate_2param _create_static(const char *name = NULL)
	{
		return proto_delegate_2param(delegate_gcc_mfp_null, reinterpret_cast<static_func>(_FunctionPtr), &deferred_cast<_FunctionClass>, name);
	}

	// equality testing
	bool operator==(const proto_delegate_2param &rhs) const
	{
		return (m_function == rhs.m_function && m_caster == rhs.m_caster &&
				m_rawfunction.u.funcptr == rhs.m_rawfunction.u.funcptr &&
				m_rawfunction.this_delta == rhs.m_rawfunction.this_delta);
	}

protected:
	// internal state
	static_func					m_function;			// generic static function pointer
	delegate_gcc_mfp_internal	m_rawfunction;		// raw member function definition
};


// ======================> delegate_2param

template<typename _ReturnType, typename _P1Type, typename _P2Type>
class delegate_2param : public proto_delegate_2param<_ReturnType, _P1Type, _P2Type>
{
	typedef _ReturnType (*static_func)(delegate_generic_class *, _P1Type, _P2Type);
	typedef proto_delegate_2param<_ReturnType, _P1Type, _P2Type> proto_base;

	using delegate_base::m_caster;
	using proto_base::m_function;
	using proto_base::m_rawfunction;

public:
	// constructors
	delegate_2param()
		: m_object(NULL) { }

	delegate_2param(proto_base proto)
		: proto_delegate_2param<_ReturnType, _P1Type, _P2Type>(proto),
		  m_object(NULL) { }

	delegate_2param(proto_base proto, bindable_object &object)
		: proto_delegate_2param<_ReturnType, _P1Type, _P2Type>(proto),
		  m_object(NULL) { bind(object); }

	// bind the actual object
	void bind(bindable_object &object)
	{
		m_object = (*m_caster)(object);
		if (m_rawfunction.u.funcptr != NULL)
			m_function = reinterpret_cast<static_func>(delegate_convert_raw(m_object, m_rawfunction));
	}

	// call the function
	_ReturnType operator()(_P1Type p1, _P2Type p2) const { return (*m_function)(m_object, p1, p2); }

	// testing
	bool has_object() const { return (m_object != NULL); }
	bool operator==(const delegate_2param &rhs) const { return (m_object == rhs.m_object && proto_base::operator==(rhs)); }

protected:
	// internal state
	delegate_generic_class *m_object;			// pointer to the post-cast object
};


// ======================> proto_delegate_3param

template<typename _ReturnType, typename _P1Type, typename _P2Type, typename _P3Type>
class proto_delegate_3param : public delegate_base
{
protected:
	// pointer to a static function which takes the object as a first parameter
	typedef _ReturnType (*static_func)(delegate_generic_class *, _P1Type, _P2Type, _P3Type);

public:
	// constructors
	proto_delegate_3param(delegate_gcc_mfp_internal &mfp = delegate_gcc_mfp_null, static_func function = NULL, deferred_cast_func caster = NULL, const char *name = NULL)
		: delegate_base(caster, name),
		  m_function(function),
		  m_rawfunction(mfp) { }

	proto_delegate_3param(const proto_delegate_3param &proto)
		: delegate_base(proto.m_caster, proto.m_name),
		  m_function(proto.m_function),
		  m_rawfunction(proto.m_rawfunction) { }

	// create a member function proto-delegate
	template<class _FunctionClass, _ReturnType (_FunctionClass::*_FunctionPtr)(_P1Type, _P2Type, _P3Type)>
	static proto_delegate_3param _create_member(const char *name = NULL)
	{
		union
		{
			_ReturnType (_FunctionClass::*mfp)(_P1Type, _P2Type, _P3Type);
			delegate_gcc_mfp_internal	internal;
		} tempunion;
		tempunion.mfp = _FunctionPtr;
		return proto_delegate_3param(tempunion.internal, NULL, &deferred_cast<_FunctionClass>, name);
	}

	// create a static function proto-delegate
	template<class _FunctionClass, _ReturnType (*_FunctionPtr)(_FunctionClass *, _P1Type, _P2Type, _P3Type)>
	static proto_delegate_3param _create_static(const char *name = NULL)
	{
		return proto_delegate_3param(delegate_gcc_mfp_null, reinterpret_cast<static_func>(_FunctionPtr), &deferred_cast<_FunctionClass>, name);
	}

	// equality testing
	bool operator==(const proto_delegate_3param &rhs) const
	{
		return (m_function == rhs.m_function && m_caster == rhs.m_caster &&
				m_rawfunction.u.funcptr == rhs.m_rawfunction.u.funcptr &&
				m_rawfunction.this_delta == rhs.m_rawfunction.this_delta);
	}

protected:
	// internal state
	static_func					m_function;			// generic static function pointer
	delegate_gcc_mfp_internal	m_rawfunction;		// raw member function definition
};


// ======================> delegate_3param

template<typename _ReturnType, typename _P1Type, typename _P2Type, typename _P3Type>
class delegate_3param : public proto_delegate_3param<_ReturnType, _P1Type, _P2Type, _P3Type>
{
	typedef _ReturnType (*static_func)(delegate_generic_class *, _P1Type, _P2Type, _P3Type);
	typedef proto_delegate_3param<_ReturnType, _P1Type, _P2Type, _P3Type> proto_base;

	using delegate_base::m_caster;
	using proto_base::m_function;
	using proto_base::m_rawfunction;

public:
	// constructors
	delegate_3param()
		: m_object(NULL) { }

	delegate_3param(proto_base proto)
		: proto_delegate_3param<_ReturnType, _P1Type, _P2Type, _P3Type>(proto),
		  m_object(NULL) { }

	delegate_3param(proto_base proto, bindable_object &object)
		: proto_delegate_3param<_ReturnType, _P1Type, _P2Type, _P3Type>(proto),
		  m_object(NULL) { bind(object); }

	// bind the actual object
	void bind(bindable_object &object)
	{
		m_object = (*m_caster)(object);
		if (m_rawfunction.u.funcptr != NULL)
			m_function = reinterpret_cast<static_func>(delegate_convert_raw(m_object, m_rawfunction));
	}

	// call the function
	_ReturnType operator()(_P1Type p1, _P2Type p2, _P3Type p3) const { return (*m_function)(m_object, p1, p2, p3); }

	// testing
	bool has_object() const { return (m_object != NULL); }
	bool operator==(const delegate_3param &rhs) const { return (m_object == rhs.m_object && proto_base::operator==(rhs)); }

protected:
	// internal state
	delegate_generic_class *m_object;			// pointer to the post-cast object
};


// ======================> proto_delegate_4param

template<typename _ReturnType, typename _P1Type, typename _P2Type, typename _P3Type, typename _P4Type>
class proto_delegate_4param : public delegate_base
{
protected:
	// pointer to a static function which takes the object as a first parameter
	typedef _ReturnType (*static_func)(delegate_generic_class *, _P1Type, _P2Type, _P3Type, _P4Type);

public:
	// constructors
	proto_delegate_4param(delegate_gcc_mfp_internal &mfp = delegate_gcc_mfp_null, static_func function = NULL, deferred_cast_func caster = NULL, const char *name = NULL)
		: delegate_base(caster, name),
		  m_function(function),
		  m_rawfunction(mfp) { }

	proto_delegate_4param(const proto_delegate_4param &proto)
		: delegate_base(proto.m_caster, proto.m_name),
		  m_function(proto.m_function),
		  m_rawfunction(proto.m_rawfunction) { }

	// create a member function proto-delegate
	template<class _FunctionClass, _ReturnType (_FunctionClass::*_FunctionPtr)(_P1Type, _P2Type, _P3Type, _P4Type)>
	static proto_delegate_4param _create_member(const char *name = NULL)
	{
		union
		{
			_ReturnType (_FunctionClass::*mfp)(_P1Type, _P2Type, _P3Type, _P4Type);
			delegate_gcc_mfp_internal	internal;
		} tempunion;
		tempunion.mfp = _FunctionPtr;
		return proto_delegate_4param(tempunion.internal, NULL, &deferred_cast<_FunctionClass>, name);
	}

	// create a static function proto-delegate
	template<class _FunctionClass, _ReturnType (*_FunctionPtr)(_FunctionClass *, _P1Type, _P2Type, _P3Type, _P4Type)>
	static proto_delegate_4param _create_static(const char *name = NULL)
	{
		return proto_delegate_4param(delegate_gcc_mfp_null, reinterpret_cast<static_func>(_FunctionPtr), &deferred_cast<_FunctionClass>, name);
	}

	// equality testing
	bool operator==(const proto_delegate_4param &rhs) const
	{
		return (m_function == rhs.m_function && m_caster == rhs.m_caster &&
				m_rawfunction.u.funcptr == rhs.m_rawfunction.u.funcptr &&
				m_rawfunction.this_delta == rhs.m_rawfunction.this_delta);
	}

protected:
	// internal state
	static_func					m_function;			// generic static function pointer
	delegate_gcc_mfp_internal	m_rawfunction;		// raw member function definition
};


// ======================> delegate_4param

template<typename _ReturnType, typename _P1Type, typename _P2Type, typename _P3Type, typename _P4Type>
class delegate_4param : public proto_delegate_4param<_ReturnType, _P1Type, _P2Type, _P3Type, _P4Type>
{
	typedef _ReturnType (*static_func)(delegate_generic_class *, _P1Type, _P2Type, _P3Type, _P4Type);
	typedef proto_delegate_4param<_ReturnType, _P1Type, _P2Type, _P3Type, _P4Type> proto_base;

	using delegate_base::m_caster;
	using proto_base::m_function;
	using proto_base::m_rawfunction;

public:
	// constructors
	delegate_4param()
		: m_object(NULL) { }

	delegate_4param(proto_base proto)
		: proto_delegate_4param<_ReturnType, _P1Type, _P2Type, _P3Type, _P4Type>(proto),
		  m_object(NULL) { }

	delegate_4param(proto_base proto, bindable_object &object)
		: proto_delegate_4param<_ReturnType, _P1Type, _P2Type, _P3Type, _P4Type>(proto),
		  m_object(NULL) { bind(object); }

	// bind the actual object
	void bind(bindable_object &object)
	{
		m_object = (*m_caster)(object);
		if (m_rawfunction.u.funcptr != NULL)
			m_function = reinterpret_cast<static_func>(delegate_convert_raw(m_object, m_rawfunction));
	}

	// call the function
	_ReturnType operator()(_P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4) const { return (*m_function)(m_object, p1, p2, p3, p4); }

	// testing
	bool has_object() const { return (m_object != NULL); }
	bool operator==(const delegate_4param &rhs) const { return (m_object == rhs.m_object && proto_base::operator==(rhs)); }

protected:
	// internal state
	delegate_generic_class *m_object;			// pointer to the post-cast object
};



#endif


#endif	/* __DELEGATE_H__ */
