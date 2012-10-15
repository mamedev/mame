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

// standard C++ includes
#include <exception>
#include <typeinfo>


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

// generic function type
typedef void (*delegate_generic_function)();


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


// ======================> binding_type_exception

// exception that is thrown when a bind fails the dynamic_cast
class binding_type_exception : public std::exception
{
public:
	binding_type_exception(const std::type_info &target_type, const std::type_info &actual_type)
		: m_target_type(target_type), m_actual_type(actual_type) { }
	const std::type_info &m_target_type;
	const std::type_info &m_actual_type;
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
//  DELEGATE MEMBER FUNCTION POINTER WRAPPERS
//**************************************************************************

#if (USE_DELEGATE_TYPE == DELEGATE_TYPE_COMPATIBLE)

// ======================> delegate_mfp

// delegate_mfp is a class that wraps a generic member function pointer
// in a static buffer, and can effectively recast itself back for later use;
// it hides some of the gross details involved in copying artibtrary member
// function pointers around
struct delegate_mfp
{
	// default constructor
	delegate_mfp() { memset(&m_rawdata, 0, sizeof(m_rawdata)); }
	
	// copy constructor
	delegate_mfp(const delegate_mfp &src) 
		: m_rawdata(src.m_rawdata) { }

	// construct from any member function pointer
	template<typename _FunctionType>
	delegate_mfp(_FunctionType mfp)
	{
		assert(sizeof(mfp) <= sizeof(m_rawdata));
		memset(&m_rawdata, 0, sizeof(m_rawdata));
		*reinterpret_cast<_FunctionType *>(&m_rawdata) = mfp;
	}

	// assignment operator
	delegate_mfp &operator=(const delegate_mfp &src)
	{
		if (this != &src)
			m_rawdata = src.m_rawdata;
		return *this;
	}

	// comparison operator
	bool operator==(const delegate_mfp &rhs) const
	{
		return (memcmp(&m_rawdata, &rhs.m_rawdata, sizeof(m_rawdata)) == 0);
	}

	// isnull checker
	bool isnull() const
	{
		for (int index = 0; index < ARRAY_LENGTH(m_rawdata.data); index++)
			if (m_rawdata.data[index] != 0)
				return false;
		return true;
	}

	// convert back to a member function pointer
	template<class _FunctionType>
	_FunctionType &mfp() const { return *reinterpret_cast<_FunctionType *>(&m_rawdata); }

	// for MSVC maximum size is one pointer, plus 3 ints;
	// all other implementations seem to be smaller
	static const int MAX_MFP_SIZE = sizeof(void *) + 3 * sizeof(int);

	// raw buffer to hold the copy of the function pointer
	mutable struct { int data[(MAX_MFP_SIZE + sizeof(int) - 1) / sizeof(int)]; } m_rawdata;
};

#elif (USE_DELEGATE_TYPE == DELEGATE_TYPE_INTERNAL)

// ======================> delegate_mfp

// struct describing the contents of a member function pointer
struct delegate_mfp
{
	// default constructor
	delegate_mfp()
		: m_function(0),
		  m_this_delta(0) { }

	// copy constructor
	delegate_mfp(const delegate_mfp &src)
		: m_function(src.m_function),
		  m_this_delta(src.m_this_delta) { }

	// construct from any member function pointer
	template<typename _FunctionPtr>
	delegate_mfp(_FunctionPtr mfp)
	{
		assert(sizeof(mfp) == sizeof(*this));
		*reinterpret_cast<_FunctionPtr *>(this) = mfp;
	}

	// assignment operator
	delegate_mfp &operator=(const delegate_mfp &src)
	{
		if (this != &src)
		{
			m_function = src.m_function;
			m_this_delta = src.m_this_delta;
		}
		return *this;
	}

	// comparison operator
	bool operator==(const delegate_mfp &rhs) const
	{
		return (m_function == rhs.m_function && m_this_delta == rhs.m_this_delta);
	}
	
	// isnull checker
	bool isnull() const { return (m_function == 0); }

	// extract the generic function and adjust the object pointer
	delegate_generic_function convert_to_generic(delegate_generic_class *&object) const;

	// actual state
	FPTR					m_function;			// first item can be one of two things:
												//    if even, it's a pointer to the function
												//    if odd, it's the byte offset into the vtable
	int 					m_this_delta;		// delta to apply to the 'this' pointer
};

#endif


//**************************************************************************
//  COMMON DELEGATE BASE CLASS
//**************************************************************************

// ======================> delegate_common_base

// common non-templatized base class
class delegate_common_base
{
protected:
	typedef delegate_generic_class *(*late_bind_func)(delegate_late_bind &object);

	// construction
	delegate_common_base(const char *name = NULL, late_bind_func latebinder = NULL, delegate_generic_function funcptr = NULL)
		: m_name(name),
		  m_object(NULL),
		  m_latebinder(latebinder),
		  m_raw_function(funcptr) { }

	template<typename _FunctionPtr>
	delegate_common_base(const char *name, late_bind_func latebinder, _FunctionPtr funcptr)
		: m_name(name),
		  m_object(NULL),
		  m_latebinder(latebinder),
		  m_raw_function(NULL),
		  m_raw_mfp(funcptr) { }

	// copy constructor
	delegate_common_base(const delegate_common_base &src)
		: m_name(src.m_name),
		  m_object(src.m_object),
		  m_latebinder(src.m_latebinder),
		  m_raw_function(src.m_raw_function),
		  m_raw_mfp(src.m_raw_mfp) { }

	// copy helper
	void copy(const delegate_common_base &src)
	{
		m_name = src.m_name;
		m_object = src.m_object;
		m_latebinder = src.m_latebinder;
		m_raw_function = src.m_raw_function;
		m_raw_mfp = src.m_raw_mfp;
	}
	
public:
	// getters
	bool has_object() const { return (m_object != NULL); }
	const char *name() const { return m_name; }
	
	// helpers
	bool isnull() const { return (m_raw_function == NULL && m_raw_mfp.isnull()); }
	bool is_mfp() const { return !m_raw_mfp.isnull(); }

	// comparison helper
	bool operator==(const delegate_common_base &rhs) const
	{
		return (m_object == rhs.m_object && m_raw_function == rhs.m_raw_function && m_raw_mfp == rhs.m_raw_mfp);
	}

protected:
	// late binding helper
	template<class _FunctionClass>
	static delegate_generic_class *late_bind_helper(delegate_late_bind &object)
	{
		_FunctionClass *result = dynamic_cast<_FunctionClass *>(&object);
		if (result == NULL)
			throw binding_type_exception(typeid(_FunctionClass), typeid(object));
		return reinterpret_cast<delegate_generic_class *>(result);
	}

#if (USE_DELEGATE_TYPE == DELEGATE_TYPE_COMPATIBLE)
	// helper stub that calls the member function; we need one for each parameter count
	template<class _FunctionClass, typename _ReturnType>
    static _ReturnType method_stub(delegate_generic_class *object)
    {
    	typedef _ReturnType (_FunctionClass::*mfptype)();
    	delegate_common_base *_this = reinterpret_cast<delegate_common_base *>(object);
    	mfptype &mfp = _this->m_raw_mfp.mfp<mfptype>();
    	return (reinterpret_cast<_FunctionClass *>(_this->m_object)->*mfp)();
    }

	template<class _FunctionClass, typename _ReturnType, typename _P1Type>
    static _ReturnType method_stub(delegate_generic_class *object, _P1Type p1)
    {
    	typedef _ReturnType (_FunctionClass::*mfptype)(_P1Type p1);
    	delegate_common_base *_this = reinterpret_cast<delegate_common_base *>(object);
    	mfptype &mfp = _this->m_raw_mfp.mfp<mfptype>();
    	return (reinterpret_cast<_FunctionClass *>(_this->m_object)->*mfp)(p1);
    }

	template<class _FunctionClass, typename _ReturnType, typename _P1Type, typename _P2Type>
    static _ReturnType method_stub(delegate_generic_class *object, _P1Type p1, _P2Type p2)
    {
    	typedef _ReturnType (_FunctionClass::*mfptype)(_P1Type p1, _P2Type p2);
    	delegate_common_base *_this = reinterpret_cast<delegate_common_base *>(object);
    	mfptype &mfp = _this->m_raw_mfp.mfp<mfptype>();
    	return (reinterpret_cast<_FunctionClass *>(_this->m_object)->*mfp)(p1, p2);
    }

	template<class _FunctionClass, typename _ReturnType, typename _P1Type, typename _P2Type, typename _P3Type>
    static _ReturnType method_stub(delegate_generic_class *object, _P1Type p1, _P2Type p2, _P3Type p3)
    {
    	typedef _ReturnType (_FunctionClass::*mfptype)(_P1Type p1, _P2Type p2, _P3Type p3);
    	delegate_common_base *_this = reinterpret_cast<delegate_common_base *>(object);
    	mfptype &mfp = _this->m_raw_mfp.mfp<mfptype>();
    	return (reinterpret_cast<_FunctionClass *>(_this->m_object)->*mfp)(p1, p2, p3);
    }

	template<class _FunctionClass, typename _ReturnType, typename _P1Type, typename _P2Type, typename _P3Type, typename _P4Type>
    static _ReturnType method_stub(delegate_generic_class *object, _P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4)
    {
    	typedef _ReturnType (_FunctionClass::*mfptype)(_P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4);
    	delegate_common_base *_this = reinterpret_cast<delegate_common_base *>(object);
    	mfptype &mfp = _this->m_raw_mfp.mfp<mfptype>();
    	return (reinterpret_cast<_FunctionClass *>(_this->m_object)->*mfp)(p1, p2, p3, p4);
    }

	template<class _FunctionClass, typename _ReturnType, typename _P1Type, typename _P2Type, typename _P3Type, typename _P4Type, typename _P5Type>
    static _ReturnType method_stub(delegate_generic_class *object, _P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4, _P5Type p5)
    {
    	typedef _ReturnType (_FunctionClass::*mfptype)(_P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4, _P5Type p5);
    	delegate_common_base *_this = reinterpret_cast<delegate_common_base *>(object);
    	mfptype &mfp = _this->m_raw_mfp.mfp<mfptype>();
    	return (reinterpret_cast<_FunctionClass *>(_this->m_object)->*mfp)(p1, p2, p3, p4, p5);
    }
#endif

	// internal state
	const char *				m_name;				// name string
	delegate_generic_class *	m_object;			// pointer to the post-cast object
	late_bind_func				m_latebinder;		// late binding helper
	delegate_generic_function	m_raw_function;		// raw static function pointer
	delegate_mfp				m_raw_mfp;			// raw member function pointer
};



//**************************************************************************
//  TEMPLATIZED DELEGATE BASE
//**************************************************************************

// ======================> delegate_base

// general delegate class template supporting up to 4 parameters
template<typename _ReturnType, typename _P1Type = _noparam, typename _P2Type = _noparam, typename _P3Type = _noparam, typename _P4Type = _noparam, typename _P5Type = _noparam>
class delegate_base : public delegate_common_base
{
#if (USE_DELEGATE_TYPE == DELEGATE_TYPE_COMPATIBLE)
	delegate_generic_class *copy_callobject(const delegate_base &src) { return src.is_mfp() ? reinterpret_cast<delegate_generic_class *>(this) : src.m_object; }
#else
	delegate_generic_class *copy_callobject(const delegate_base &src) { return src.m_callobject; }
#endif

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
		: m_function(NULL),
		  m_callobject(NULL) { }

	// copy constructor
	delegate_base(const delegate_base &src)
		: delegate_common_base(src),
		  m_function(src.m_function),
		  m_callobject(copy_callobject(src)) { }

	// copy constructor with late bind
	delegate_base(const delegate_base &src, delegate_late_bind &object)
		: delegate_common_base(src),
		  m_function(src.m_function),
		  m_callobject(copy_callobject(src))
	{
		late_bind(object);
	}

	// construct from member function with object pointer
	template<class _FunctionClass>
	delegate_base(typename traits<_FunctionClass>::member_func_type funcptr, const char *name, _FunctionClass *object)
		: delegate_common_base(name, &late_bind_helper<_FunctionClass>, funcptr),
#if (USE_DELEGATE_TYPE == DELEGATE_TYPE_COMPATIBLE)
		  m_function(&delegate_base::method_stub<_FunctionClass, _ReturnType>),
		  m_callobject(reinterpret_cast<delegate_generic_class *>(this))
#else
		  m_function(NULL)
#endif
	{
		bind(reinterpret_cast<delegate_generic_class *>(object));
	}

	// construct from static function with object pointer
	template<class _FunctionClass>
	delegate_base(typename traits<_FunctionClass>::static_func_type funcptr, const char *name, _FunctionClass *object)
		: delegate_common_base(name, &late_bind_helper<_FunctionClass>, reinterpret_cast<delegate_generic_function>(funcptr)),
		  m_function(reinterpret_cast<generic_static_func>(funcptr)),
		  m_callobject(NULL)
	{
		bind(reinterpret_cast<delegate_generic_class *>(object));
	}

	// construct from static reference function with object pointer
	template<class _FunctionClass>
	delegate_base(typename traits<_FunctionClass>::static_ref_func_type funcptr, const char *name, _FunctionClass *object)
		: delegate_common_base(name, &late_bind_helper<_FunctionClass>, reinterpret_cast<delegate_generic_function>(funcptr)),
		  m_function(reinterpret_cast<generic_static_func>(funcptr)),
		  m_callobject(NULL)
	{
		bind(reinterpret_cast<delegate_generic_class *>(object));
	}

	// copy operator
	delegate_base &operator=(const delegate_base &src)
	{
		if (this != &src)
		{
			delegate_common_base::copy(src);
			m_callobject = copy_callobject(src);
			m_function = src.m_function;
		}
		return *this;
	}

	// call the function
#if (USE_DELEGATE_TYPE == DELEGATE_TYPE_COMPATIBLE)
	_ReturnType operator()() const { return (*m_function)(m_callobject); }
	_ReturnType operator()(_P1Type p1) const { return (*m_function)(m_callobject, p1); }
	_ReturnType operator()(_P1Type p1, _P2Type p2) const { return (*m_function)(m_callobject, p1, p2); }
	_ReturnType operator()(_P1Type p1, _P2Type p2, _P3Type p3) const { return (*m_function)(m_callobject, p1, p2, p3); }
	_ReturnType operator()(_P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4) const { return (*m_function)(m_callobject, p1, p2, p3, p4); }
	_ReturnType operator()(_P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4, _P5Type p5) const { return (*m_function)(m_callobject, p1, p2, p3, p4, p5); }
#else
	_ReturnType operator()() const { return (*m_function)(m_object); }
	_ReturnType operator()(_P1Type p1) const { return (*m_function)(m_object, p1); }
	_ReturnType operator()(_P1Type p1, _P2Type p2) const { return (*m_function)(m_object, p1, p2); }
	_ReturnType operator()(_P1Type p1, _P2Type p2, _P3Type p3) const { return (*m_function)(m_object, p1, p2, p3); }
	_ReturnType operator()(_P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4) const { return (*m_function)(m_object, p1, p2, p3, p4); }
	_ReturnType operator()(_P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4, _P5Type p5) const { return (*m_function)(m_object, p1, p2, p3, p4, p5); }
#endif

	// late binding
	void late_bind(delegate_late_bind &object) { bind((*m_latebinder)(object)); }

protected:
	// bind the actual object
	void bind(delegate_generic_class *object)
	{
		m_object = object;
		
		// update callobject to match, unless it is pointing to ourself
		if (m_callobject != reinterpret_cast<delegate_generic_class *>(this))
			m_callobject = m_object;

#if (USE_DELEGATE_TYPE != DELEGATE_TYPE_COMPATIBLE)
		// update the function
		if (m_object != NULL && is_mfp())
			m_function = reinterpret_cast<generic_static_func>(m_raw_mfp.convert_to_generic(m_object));
#endif
	}

	// internal state
	generic_static_func			m_function;			// generic static function pointer
	delegate_generic_class *	m_callobject;		// pointer to the object used for calling
};



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
