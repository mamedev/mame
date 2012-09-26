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


// ======================> delegate_late_bind

// simple polymorphic class that must be mixed into any object that is late-bound
class delegate_late_bind
{
public:
	delegate_late_bind() { }
	virtual ~delegate_late_bind() { }
};


// ======================> delegate_traits

// delegate_traits is a meta-template that is used to provide a static function pointer
// and member function pointer of the appropriate type and number of parameters; we use
// partial template specialization to support fewer parameters by defaulting the later
// parameters to the special type _noparam
template<typename _ClassType, typename _ReturnType, typename _P1Type, typename _P2Type, typename _P3Type, typename _P4Type, typename _P5Type>
struct delegate_traits
{
	typedef _ReturnType (*static_func_type)(_ClassType *, _P1Type, _P2Type, _P3Type, _P4Type, _P5Type);
	typedef _ReturnType (*static_ref_func_type)(_ClassType &, _P1Type, _P2Type, _P3Type, _P4Type, _P5Type);
	typedef _ReturnType (_ClassType::*member_func_type)(_P1Type, _P2Type, _P3Type, _P4Type, _P5Type);
};

// dummy class used to indicate a non-existant parameter
class _noparam { };

// specialization for 4 parameters
template<typename _ClassType, typename _ReturnType, typename _P1Type, typename _P2Type, typename _P3Type, typename _P4Type>
struct delegate_traits<_ClassType, _ReturnType, _P1Type, _P2Type, _P3Type, _P4Type, _noparam>
{
	typedef _ReturnType (*static_func_type)(_ClassType *, _P1Type, _P2Type, _P3Type, _P4Type);
	typedef _ReturnType (*static_ref_func_type)(_ClassType &, _P1Type, _P2Type, _P3Type, _P4Type);
	typedef _ReturnType (_ClassType::*member_func_type)(_P1Type, _P2Type, _P3Type, _P4Type);
};

// specialization for 3 parameters
template<typename _ClassType, typename _ReturnType, typename _P1Type, typename _P2Type, typename _P3Type>
struct delegate_traits<_ClassType, _ReturnType, _P1Type, _P2Type, _P3Type, _noparam, _noparam>
{
	typedef _ReturnType (*static_func_type)(_ClassType *, _P1Type, _P2Type, _P3Type);
	typedef _ReturnType (*static_ref_func_type)(_ClassType &, _P1Type, _P2Type, _P3Type);
	typedef _ReturnType (_ClassType::*member_func_type)(_P1Type, _P2Type, _P3Type);
};

// specialization for 2 parameters
template<typename _ClassType, typename _ReturnType, typename _P1Type, typename _P2Type>
struct delegate_traits<_ClassType, _ReturnType, _P1Type, _P2Type, _noparam, _noparam, _noparam>
{
	typedef _ReturnType (*static_func_type)(_ClassType *, _P1Type, _P2Type);
	typedef _ReturnType (*static_ref_func_type)(_ClassType &, _P1Type, _P2Type);
	typedef _ReturnType (_ClassType::*member_func_type)(_P1Type, _P2Type);
};

// specialization for 1 parameter
template<typename _ClassType, typename _ReturnType, typename _P1Type>
struct delegate_traits<_ClassType, _ReturnType, _P1Type, _noparam, _noparam, _noparam, _noparam>
{
	typedef _ReturnType (*static_func_type)(_ClassType *, _P1Type);
	typedef _ReturnType (*static_ref_func_type)(_ClassType &, _P1Type);
	typedef _ReturnType (_ClassType::*member_func_type)(_P1Type);
};

// specialization for no parameters
template<typename _ClassType, typename _ReturnType>
struct delegate_traits<_ClassType, _ReturnType, _noparam, _noparam, _noparam, _noparam, _noparam>
{
	typedef _ReturnType (*static_func_type)(_ClassType *);
	typedef _ReturnType (*static_ref_func_type)(_ClassType &);
	typedef _ReturnType (_ClassType::*member_func_type)();
};



//**************************************************************************
//  COMPATIBLE DELEGATES
//**************************************************************************

#if (USE_DELEGATE_TYPE == DELEGATE_TYPE_COMPATIBLE)

// ======================> delegate_raw_mfp

// delegate_raw_mfp is a class that wraps a generic member function pointer
// in a static buffer, and can effectively recast itself back for later use;
// it hides some of the gross details involved in copying artibtrary member
// function pointers around
struct delegate_raw_mfp
{
	// for MSVC maximum size is one pointer, plus 3 ints
	static const int MAX_MFP_SIZE = sizeof(void *) + 3 * sizeof(int);

	// default and copy constructors
	delegate_raw_mfp() { memset(&m_rawdata, 0, sizeof(m_rawdata)); }
	delegate_raw_mfp(const delegate_raw_mfp &src) : m_rawdata(src.m_rawdata) { }

	// construct from any member function pointer
	template<typename _FunctionType>
	delegate_raw_mfp(_FunctionType mfp)
	{
		assert(sizeof(mfp) <= sizeof(m_rawdata));
		memset(&m_rawdata, 0, sizeof(m_rawdata));
		*reinterpret_cast<_FunctionType *>(&m_rawdata) = mfp;
	}

	// assignment operator
	delegate_raw_mfp &operator=(const delegate_raw_mfp &src)
	{
		if (this != &src)
			m_rawdata = src.m_rawdata;
		return *this;
	}

	// comparison operator
	bool operator==(const delegate_raw_mfp &rhs) const
	{
		return (memcmp(&m_rawdata, &rhs.m_rawdata, sizeof(m_rawdata)) == 0);
	}

	// convert back to a member function pointer
	template<class _FunctionType>
	_FunctionType &mfp() const { return *reinterpret_cast<_FunctionType *>(&m_rawdata); }

	// raw buffer to hold the copy of the function pointer
	mutable struct { UINT8 bytes[MAX_MFP_SIZE]; } m_rawdata;
};


// ======================> delegate_base

// general delegate class template supporting up to 4 parameters
template<typename _ReturnType, typename _P1Type = _noparam, typename _P2Type = _noparam, typename _P3Type = _noparam, typename _P4Type = _noparam, typename _P5Type = _noparam>
class delegate_base
{
	typedef delegate_generic_class *(*late_bind_func)(delegate_late_bind &object);

public:
	// define our traits
	template<class _FunctionClass>
	struct traits
	{
		typedef typename delegate_traits<_FunctionClass, _ReturnType, _P1Type, _P2Type, _P3Type, _P4Type, _P5Type>::member_func_type member_func_type;
		typedef typename delegate_traits<_FunctionClass, _ReturnType, _P1Type, _P2Type, _P3Type, _P4Type, _P5Type>::static_func_type static_func_type;
		typedef typename delegate_traits<_FunctionClass, _ReturnType, _P1Type, _P2Type, _P3Type, _P4Type, _P5Type>::static_ref_func_type static_ref_func_type;
	};
	typedef typename traits<delegate_generic_class>::static_func_type generic_static_func;

	// generic constructor
	delegate_base()
		: m_name(NULL),
		  m_object(NULL),
		  m_callobject(NULL),
		  m_function(NULL),
		  m_latebinder(NULL) { }

	// copy constructor
	delegate_base(const delegate_base &src)
		: m_name(src.m_name),
		  m_object(src.m_object),
		  m_callobject(src.is_mfp() ? reinterpret_cast<delegate_generic_class *>(this) : src.m_object),
		  m_function(src.m_function),
		  m_rawfunction(src.m_rawfunction),
		  m_latebinder(src.m_latebinder) { }

	// copy constructor with late bind
	delegate_base(const delegate_base &src, delegate_late_bind &object)
		: m_name(src.m_name),
		  m_object(src.m_object),
		  m_callobject(src.is_mfp() ? reinterpret_cast<delegate_generic_class *>(this) : src.m_object),
		  m_function(src.m_function),
		  m_rawfunction(src.m_rawfunction),
		  m_latebinder(src.m_latebinder) { late_bind(object); }

	// construct from member function with object pointer
	template<class _FunctionClass>
	delegate_base(typename traits<_FunctionClass>::member_func_type funcptr, const char *name, _FunctionClass *object)
		: m_name(name),
		  m_object(NULL),
		  m_callobject(reinterpret_cast<delegate_generic_class *>(this)),
		  m_function(&delegate_base::method_stub<_FunctionClass>),
		  m_rawfunction(funcptr),
		  m_latebinder(&late_bind_helper<_FunctionClass>) { bind(reinterpret_cast<delegate_generic_class *>(object)); }

	// construct from static function with object pointer
	template<class _FunctionClass>
	delegate_base(typename traits<_FunctionClass>::static_func_type funcptr, const char *name, _FunctionClass *object)
		: m_name(name),
		  m_object(NULL),
		  m_callobject(NULL),
		  m_function(reinterpret_cast<generic_static_func>(funcptr)),
		  m_latebinder(&late_bind_helper<_FunctionClass>) { bind(reinterpret_cast<delegate_generic_class *>(object)); }

	// construct from static reference function with object pointer
	template<class _FunctionClass>
	delegate_base(typename traits<_FunctionClass>::static_ref_func_type funcptr, const char *name, _FunctionClass *object)
		: m_name(name),
		  m_object(NULL),
		  m_callobject(NULL),
		  m_function(reinterpret_cast<generic_static_func>(funcptr)),
		  m_latebinder(&late_bind_helper<_FunctionClass>) { bind(reinterpret_cast<delegate_generic_class *>(object)); }

	// copy operator
	delegate_base &operator=(const delegate_base &src)
	{
		if (this != &src)
		{
			m_name = src.m_name;
			m_object = src.m_object;
			m_callobject = src.is_mfp() ? reinterpret_cast<delegate_generic_class *>(this) : src.m_object;
			m_function = src.m_function;
			m_rawfunction = src.m_rawfunction;
			m_latebinder = src.m_latebinder;
		}
		return *this;
	}

	// comparison operator
	bool operator==(const delegate_base &rhs) const
	{
		return (m_object == rhs.m_object && m_function == rhs.m_function && m_rawfunction == rhs.m_rawfunction);
	}

	// getters
	bool isnull() const { return (m_function == NULL); }
	bool has_object() const { return (m_object != NULL); }
	const char *name() const { return m_name; }

	// call the function
	_ReturnType operator()() const { return (*m_function)(m_callobject); }
	_ReturnType operator()(_P1Type p1) const { return (*m_function)(m_callobject, p1); }
	_ReturnType operator()(_P1Type p1, _P2Type p2) const { return (*m_function)(m_callobject, p1, p2); }
	_ReturnType operator()(_P1Type p1, _P2Type p2, _P3Type p3) const { return (*m_function)(m_callobject, p1, p2, p3); }
	_ReturnType operator()(_P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4) const { return (*m_function)(m_callobject, p1, p2, p3, p4); }
	_ReturnType operator()(_P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4, _P5Type p5) const { return (*m_function)(m_callobject, p1, p2, p3, p4, p5); }

	// late binding
	void late_bind(delegate_late_bind &object) { bind((*m_latebinder)(object)); }

protected:
	// bind the actual object
	void bind(delegate_generic_class *object)
	{
		m_object = object;
		if (!is_mfp()) m_callobject = m_object;
	}

	// internal helpers
	bool is_mfp() const { return m_callobject == reinterpret_cast<const delegate_generic_class *>(this); }

	// helper stub that calls the member function; we need one for each parameter count
	template<class _FunctionClass>
    static _ReturnType method_stub(delegate_generic_class *object)
    {
    	typedef _ReturnType (_FunctionClass::*mfptype)();
    	delegate_base *_this = reinterpret_cast<delegate_base *>(object);
    	mfptype &mfp = _this->m_rawfunction.mfp<mfptype>();
    	return (reinterpret_cast<_FunctionClass *>(_this->m_object)->*mfp)();
    }

	template<class _FunctionClass>
    static _ReturnType method_stub(delegate_generic_class *object, _P1Type p1)
    {
    	typedef _ReturnType (_FunctionClass::*mfptype)(_P1Type p1);
    	delegate_base *_this = reinterpret_cast<delegate_base *>(object);
    	mfptype &mfp = _this->m_rawfunction.mfp<mfptype>();
    	return (reinterpret_cast<_FunctionClass *>(_this->m_object)->*mfp)(p1);
    }

	template<class _FunctionClass>
    static _ReturnType method_stub(delegate_generic_class *object, _P1Type p1, _P2Type p2)
    {
    	typedef _ReturnType (_FunctionClass::*mfptype)(_P1Type p1, _P2Type p2);
    	delegate_base *_this = reinterpret_cast<delegate_base *>(object);
    	mfptype &mfp = _this->m_rawfunction.mfp<mfptype>();
    	return (reinterpret_cast<_FunctionClass *>(_this->m_object)->*mfp)(p1, p2);
    }

	template<class _FunctionClass>
    static _ReturnType method_stub(delegate_generic_class *object, _P1Type p1, _P2Type p2, _P3Type p3)
    {
    	typedef _ReturnType (_FunctionClass::*mfptype)(_P1Type p1, _P2Type p2, _P3Type p3);
    	delegate_base *_this = reinterpret_cast<delegate_base *>(object);
    	mfptype &mfp = _this->m_rawfunction.mfp<mfptype>();
    	return (reinterpret_cast<_FunctionClass *>(_this->m_object)->*mfp)(p1, p2, p3);
    }

	template<class _FunctionClass>
    static _ReturnType method_stub(delegate_generic_class *object, _P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4)
    {
    	typedef _ReturnType (_FunctionClass::*mfptype)(_P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4);
    	delegate_base *_this = reinterpret_cast<delegate_base *>(object);
    	mfptype &mfp = _this->m_rawfunction.mfp<mfptype>();
    	return (reinterpret_cast<_FunctionClass *>(_this->m_object)->*mfp)(p1, p2, p3, p4);
    }

	template<class _FunctionClass>
    static _ReturnType method_stub(delegate_generic_class *object, _P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4, _P5Type p5)
    {
    	typedef _ReturnType (_FunctionClass::*mfptype)(_P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4, _P5Type p5);
    	delegate_base *_this = reinterpret_cast<delegate_base *>(object);
    	mfptype &mfp = _this->m_rawfunction.mfp<mfptype>();
    	return (reinterpret_cast<_FunctionClass *>(_this->m_object)->*mfp)(p1, p2, p3, p4, p5);
    }

	// late binding helper
	template<class _FunctionClass>
	static delegate_generic_class *late_bind_helper(delegate_late_bind &object)
	{
		_FunctionClass *result = dynamic_cast<_FunctionClass *>(&object);
		return reinterpret_cast<delegate_generic_class *>(result);
	}

	// internal state
	const char *				m_name;				// name string
	delegate_generic_class *	m_object;			// pointer to the post-cast object
	delegate_generic_class *	m_callobject;		// pointer to the object used for calling
	generic_static_func			m_function;			// generic static function pointer
	delegate_raw_mfp			m_rawfunction;		// copy of raw MFP
	late_bind_func				m_latebinder;		// late binding helper
};

#endif


//**************************************************************************
//  GCC DELEGATES
//**************************************************************************

#if (USE_DELEGATE_TYPE == DELEGATE_TYPE_INTERNAL)

// a generic function pointer type and a generic member function pointer type
typedef void (*delegate_generic_function)();

// struct describing the contents of a member function pointer
struct delegate_internal_mfp
{
	// default constructor
	delegate_internal_mfp()
		: m_function(0),
		  m_this_delta(0) { }

	// copy constructor
	delegate_internal_mfp(const delegate_internal_mfp &src)
		: m_function(src.m_function),
		  m_this_delta(src.m_this_delta) { }

	// construct from any member function pointer
	template<typename _FunctionPtr>
	delegate_internal_mfp(_FunctionPtr funcptr)
	{
		assert(sizeof(funcptr) == sizeof(*this));
		*reinterpret_cast<_FunctionPtr *>(this) = funcptr;
	}

	// assignment operator
	delegate_internal_mfp &operator=(const delegate_internal_mfp &src)
	{
		if (this != &src)
		{
			m_function = src.m_function;
			m_this_delta = src.m_this_delta;
		}
		return *this;
	}

	// comparison operator
	bool operator==(const delegate_internal_mfp &rhs) const
	{
		return (m_function == rhs.m_function && m_this_delta == rhs.m_this_delta);
	}

	// extract the generic function and adjust the object pointer
	delegate_generic_function convert_to_generic(delegate_generic_class *&object) const;

	// actual state
	FPTR					m_function;			// first item can be one of two things:
												//    if even, it's a pointer to the function
												//    if odd, it's the byte offset into the vtable
	int 					m_this_delta;		// delta to apply to the 'this' pointer
};


// ======================> delegate_base

template<typename _ReturnType, typename _P1Type = _noparam, typename _P2Type = _noparam, typename _P3Type = _noparam, typename _P4Type = _noparam, typename _P5Type = _noparam>
class delegate_base
{
	typedef delegate_generic_class *(*late_bind_func)(delegate_late_bind &object);

public:
	// define our traits
	template<class _FunctionClass>
	struct traits
	{
		typedef typename delegate_traits<_FunctionClass, _ReturnType, _P1Type, _P2Type, _P3Type, _P4Type, _P5Type>::member_func_type member_func_type;
		typedef typename delegate_traits<_FunctionClass, _ReturnType, _P1Type, _P2Type, _P3Type, _P4Type, _P5Type>::static_func_type static_func_type;
		typedef typename delegate_traits<_FunctionClass, _ReturnType, _P1Type, _P2Type, _P3Type, _P4Type, _P5Type>::static_ref_func_type static_ref_func_type;
	};
	typedef typename traits<delegate_generic_class>::static_func_type generic_static_func;

	// generic constructor
	delegate_base()
		: m_name(NULL),
		  m_object(NULL),
		  m_function(NULL),
		  m_latebinder(NULL) { }

	// copy constructor
	delegate_base(const delegate_base &src)
		: m_name(src.m_name),
		  m_object(src.m_object),
		  m_function(src.m_function),
		  m_rawfunction(src.m_rawfunction),
		  m_latebinder(src.m_latebinder) { }

	// copy constructor with late bind
	delegate_base(const delegate_base &src, delegate_late_bind &object)
		: m_name(src.m_name),
		  m_object(src.m_object),
		  m_function(src.m_function),
		  m_rawfunction(src.m_rawfunction),
		  m_latebinder(src.m_latebinder) { late_bind(object); }

	// construct from member function with object pointer
	template<class _FunctionClass>
	delegate_base(typename traits<_FunctionClass>::member_func_type funcptr, const char *name, _FunctionClass *object)
		: m_name(name),
		  m_object(NULL),
		  m_function(NULL),
		  m_rawfunction(funcptr),
		  m_latebinder(&late_bind_helper<_FunctionClass>) { bind(reinterpret_cast<delegate_generic_class *>(object)); }

	// construct from static function with object pointer
	template<class _FunctionClass>
	delegate_base(typename traits<_FunctionClass>::static_func_type funcptr, const char *name, _FunctionClass *object)
		: m_name(name),
		  m_object(NULL),
		  m_function(reinterpret_cast<generic_static_func>(funcptr)),
		  m_latebinder(&late_bind_helper<_FunctionClass>) { bind(reinterpret_cast<delegate_generic_class *>(object)); }

	// construct from static reference function with object pointer
	template<class _FunctionClass>
	delegate_base(typename traits<_FunctionClass>::static_ref_func_type funcptr, const char *name, _FunctionClass *object)
		: m_name(name),
		  m_object(NULL),
		  m_function(reinterpret_cast<generic_static_func>(funcptr)),
		  m_latebinder(&late_bind_helper<_FunctionClass>) { bind(reinterpret_cast<delegate_generic_class *>(object)); }

	// copy operator
	delegate_base &operator=(const delegate_base &src)
	{
		if (this != &src)
		{
			m_name = src.m_name;
			m_object = src.m_object;
			m_function = src.m_function;
			m_rawfunction = src.m_rawfunction;
			m_latebinder = src.m_latebinder;
		}
		return *this;
	}

	// comparison operator
	bool operator==(const delegate_base &rhs) const
	{
		return (m_object == rhs.m_object && m_function == rhs.m_function && m_rawfunction == rhs.m_rawfunction);
	}

	// getters
	bool isnull() const { return (m_function == NULL && m_rawfunction.m_function == 0); }
	bool has_object() const { return (m_object != NULL); }
	const char *name() const { return m_name; }

	// call the function
	_ReturnType operator()() const { return (*m_function)(m_object); }
	_ReturnType operator()(_P1Type p1) const { return (*m_function)(m_object, p1); }
	_ReturnType operator()(_P1Type p1, _P2Type p2) const { return (*m_function)(m_object, p1, p2); }
	_ReturnType operator()(_P1Type p1, _P2Type p2, _P3Type p3) const { return (*m_function)(m_object, p1, p2, p3); }
	_ReturnType operator()(_P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4) const { return (*m_function)(m_object, p1, p2, p3, p4); }
	_ReturnType operator()(_P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4, _P5Type p5) const { return (*m_function)(m_object, p1, p2, p3, p4, p5); }

	// late binding
	void late_bind(delegate_late_bind &object) { bind((*m_latebinder)(object)); }

protected:
	// bind the actual object
	void bind(delegate_generic_class *object)
	{
		m_object = object;
		if (m_object != NULL && m_rawfunction.m_function != 0)
			m_function = reinterpret_cast<generic_static_func>(m_rawfunction.convert_to_generic(m_object));
	}

	// late binding helper
	template<class _FunctionClass>
	static delegate_generic_class *late_bind_helper(delegate_late_bind &object)
	{
		_FunctionClass *result = dynamic_cast<_FunctionClass *>(&object);
		return reinterpret_cast<delegate_generic_class *>(result);
	}

	// internal state
	const char *				m_name;				// name string
	delegate_generic_class *	m_object;			// pointer to the post-cast object
	generic_static_func			m_function;			// generic static function pointer
	delegate_internal_mfp		m_rawfunction;		// raw member function definition
	late_bind_func				m_latebinder;		// late binding helper
};

#endif


//**************************************************************************
//  NATURAL SYNTAX
//**************************************************************************

// declare the base template
template <typename Signature>
class delegate;

// specialize for 0 parameters; we derive from the base class and provide equivalent
// pass-through constructors for each type, as well as an assignment operator
template<typename _ReturnType>
class delegate<_ReturnType ()> : public delegate_base<_ReturnType>
{
	typedef delegate_base<_ReturnType> basetype;

public:
	// create a standard set of constructors
	delegate() : basetype() { }
	delegate(const basetype &src) : basetype(src) { }
	delegate(const basetype &src, delegate_late_bind &object) : basetype(src, object) { }
	template<class _FunctionClass> delegate(typename basetype::template traits<_FunctionClass>::member_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object) { }
	template<class _FunctionClass> delegate(typename basetype::template traits<_FunctionClass>::static_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object) { }
	template<class _FunctionClass> delegate(typename basetype::template traits<_FunctionClass>::static_ref_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object) { }
	delegate &operator=(const basetype &src) { *static_cast<basetype *>(this) = src; return *this; }
};

// specialize for 1 parameter
template<typename _ReturnType, typename _P1Type>
class delegate<_ReturnType (_P1Type)> : public delegate_base<_ReturnType, _P1Type>
{
	typedef delegate_base<_ReturnType, _P1Type> basetype;

public:
	// create a standard set of constructors
	delegate() : basetype() { }
	delegate(const basetype &src) : basetype(src) { }
	delegate(const basetype &src, delegate_late_bind &object) : basetype(src, object) { }
	template<class _FunctionClass> delegate(typename basetype::template traits<_FunctionClass>::member_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object) { }
	template<class _FunctionClass> delegate(typename basetype::template traits<_FunctionClass>::static_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object) { }
	template<class _FunctionClass> delegate(typename basetype::template traits<_FunctionClass>::static_ref_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object) { }
	delegate &operator=(const basetype &src) { *static_cast<basetype *>(this) = src; return *this; }
};

// specialize for 2 parameters
template<typename _ReturnType, typename _P1Type, typename _P2Type>
class delegate<_ReturnType (_P1Type, _P2Type)> : public delegate_base<_ReturnType, _P1Type, _P2Type>
{
	typedef delegate_base<_ReturnType, _P1Type, _P2Type> basetype;

public:
	// create a standard set of constructors
	delegate() : basetype() { }
	delegate(const basetype &src) : basetype(src) { }
	delegate(const basetype &src, delegate_late_bind &object) : basetype(src, object) { }
	template<class _FunctionClass> delegate(typename basetype::template traits<_FunctionClass>::member_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object) { }
	template<class _FunctionClass> delegate(typename basetype::template traits<_FunctionClass>::static_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object) { }
	template<class _FunctionClass> delegate(typename basetype::template traits<_FunctionClass>::static_ref_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object) { }
	delegate &operator=(const basetype &src) { *static_cast<basetype *>(this) = src; return *this; }
};

// specialize for 3 parameters
template<typename _ReturnType, typename _P1Type, typename _P2Type, typename _P3Type>
class delegate<_ReturnType (_P1Type, _P2Type, _P3Type)> : public delegate_base<_ReturnType, _P1Type, _P2Type, _P3Type>
{
	typedef delegate_base<_ReturnType, _P1Type, _P2Type, _P3Type> basetype;

public:
	// create a standard set of constructors
	delegate() : basetype() { }
	delegate(const basetype &src) : basetype(src) { }
	delegate(const basetype &src, delegate_late_bind &object) : basetype(src, object) { }
	template<class _FunctionClass> delegate(typename basetype::template traits<_FunctionClass>::member_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object) { }
	template<class _FunctionClass> delegate(typename basetype::template traits<_FunctionClass>::static_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object) { }
	template<class _FunctionClass> delegate(typename basetype::template traits<_FunctionClass>::static_ref_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object) { }
	delegate &operator=(const basetype &src) { *static_cast<basetype *>(this) = src; return *this; }
};

// specialize for 4 parameters
template<typename _ReturnType, typename _P1Type, typename _P2Type, typename _P3Type, typename _P4Type>
class delegate<_ReturnType (_P1Type, _P2Type, _P3Type, _P4Type)> : public delegate_base<_ReturnType, _P1Type, _P2Type, _P3Type, _P4Type>
{
	typedef delegate_base<_ReturnType, _P1Type, _P2Type, _P3Type, _P4Type> basetype;

public:
	// create a standard set of constructors
	delegate() : basetype() { }
	delegate(const basetype &src) : basetype(src) { }
	delegate(const basetype &src, delegate_late_bind &object) : basetype(src, object) { }
	template<class _FunctionClass> delegate(typename basetype::template traits<_FunctionClass>::member_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object) { }
	template<class _FunctionClass> delegate(typename basetype::template traits<_FunctionClass>::static_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object) { }
	template<class _FunctionClass> delegate(typename basetype::template traits<_FunctionClass>::static_ref_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object) { }
	delegate &operator=(const basetype &src) { *static_cast<basetype *>(this) = src; return *this; }
};

// specialize for 5 parameters
template<typename _ReturnType, typename _P1Type, typename _P2Type, typename _P3Type, typename _P4Type, typename _P5Type>
class delegate<_ReturnType (_P1Type, _P2Type, _P3Type, _P4Type, _P5Type)> : public delegate_base<_ReturnType, _P1Type, _P2Type, _P3Type, _P4Type, _P5Type>
{
	typedef delegate_base<_ReturnType, _P1Type, _P2Type, _P3Type, _P4Type, _P5Type> basetype;

public:
	// create a standard set of constructors
	delegate() : basetype() { }
	delegate(const basetype &src) : basetype(src) { }
	delegate(const basetype &src, delegate_late_bind &object) : basetype(src, object) { }
	template<class _FunctionClass> delegate(typename basetype::template traits<_FunctionClass>::member_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object) { }
	template<class _FunctionClass> delegate(typename basetype::template traits<_FunctionClass>::static_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object) { }
	template<class _FunctionClass> delegate(typename basetype::template traits<_FunctionClass>::static_ref_func_type funcptr, const char *name, _FunctionClass *object) : basetype(funcptr, name, object) { }
	delegate &operator=(const basetype &src) { *static_cast<basetype *>(this) = src; return *this; }
};

#endif	/* __DELEGATE_H__ */
