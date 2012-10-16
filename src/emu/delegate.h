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
class delegate_mfp
{
public:
	// default constructor
	delegate_mfp()
		: m_rawdata(s_null_mfp),
		  m_realobject(NULL),
		  m_stubfunction(NULL) { }
	
	// copy constructor
	delegate_mfp(const delegate_mfp &src) 
		: m_rawdata(src.m_rawdata),
		  m_realobject(src.m_realobject),
		  m_stubfunction(src.m_stubfunction) { }

	// construct from any member function pointer
	template<typename _MemberFunctionType, class _MemberFunctionClass, typename _ReturnType, typename _StaticFunctionType>
	delegate_mfp(_MemberFunctionType mfp, _MemberFunctionClass *, _ReturnType *, _StaticFunctionType)
		: m_rawdata(s_null_mfp),
		  m_realobject(NULL),
		  m_stubfunction(make_generic<_StaticFunctionType>(&delegate_mfp::method_stub<_MemberFunctionClass, _ReturnType>))
	{
		assert(sizeof(mfp) <= sizeof(m_rawdata));
		*reinterpret_cast<_MemberFunctionType *>(&m_rawdata) = mfp;
	}

	// comparison helpers
	bool operator==(const delegate_mfp &rhs) const { return (m_rawdata == rhs.m_rawdata); }
	bool isnull() const { return (m_rawdata == s_null_mfp); }
	
	// getters
	delegate_generic_class *real_object(delegate_generic_class *original) const { return m_realobject; }
	
	// binding helper
	template<typename _FunctionType>
	void update_after_bind(_FunctionType &funcptr, delegate_generic_class *&object)
	{
		m_realobject = object;
		object = reinterpret_cast<delegate_generic_class *>(this);
		funcptr = reinterpret_cast<_FunctionType>(m_stubfunction);
	}

private:
	// helper stubs for calling encased member function pointers
	template<class _FunctionClass, typename _ReturnType>
    static _ReturnType method_stub(delegate_generic_class *object)
    {
    	delegate_mfp *_this = reinterpret_cast<delegate_mfp *>(object);
    	typedef _ReturnType (_FunctionClass::*mfptype)();
    	mfptype &mfp = *reinterpret_cast<mfptype *>(&_this->m_rawdata);
    	return (reinterpret_cast<_FunctionClass *>(_this->m_realobject)->*mfp)();
    }

	template<class _FunctionClass, typename _ReturnType, typename _P1Type>
    static _ReturnType method_stub(delegate_generic_class *object, _P1Type p1)
    {
    	delegate_mfp *_this = reinterpret_cast<delegate_mfp *>(object);
    	typedef _ReturnType (_FunctionClass::*mfptype)(_P1Type p1);
    	mfptype &mfp = *reinterpret_cast<mfptype *>(&_this->m_rawdata);
    	return (reinterpret_cast<_FunctionClass *>(_this->m_realobject)->*mfp)(p1);
    }

	template<class _FunctionClass, typename _ReturnType, typename _P1Type, typename _P2Type>
    static _ReturnType method_stub(delegate_generic_class *object, _P1Type p1, _P2Type p2)
    {
    	delegate_mfp *_this = reinterpret_cast<delegate_mfp *>(object);
    	typedef _ReturnType (_FunctionClass::*mfptype)(_P1Type p1, _P2Type p2);
    	mfptype &mfp = *reinterpret_cast<mfptype *>(&_this->m_rawdata);
    	return (reinterpret_cast<_FunctionClass *>(_this->m_realobject)->*mfp)(p1, p2);
    }

	template<class _FunctionClass, typename _ReturnType, typename _P1Type, typename _P2Type, typename _P3Type>
    static _ReturnType method_stub(delegate_generic_class *object, _P1Type p1, _P2Type p2, _P3Type p3)
    {
    	delegate_mfp *_this = reinterpret_cast<delegate_mfp *>(object);
    	typedef _ReturnType (_FunctionClass::*mfptype)(_P1Type p1, _P2Type p2, _P3Type p3);
    	mfptype &mfp = *reinterpret_cast<mfptype *>(&_this->m_rawdata);
    	return (reinterpret_cast<_FunctionClass *>(_this->m_realobject)->*mfp)(p1, p2, p3);
    }

	template<class _FunctionClass, typename _ReturnType, typename _P1Type, typename _P2Type, typename _P3Type, typename _P4Type>
    static _ReturnType method_stub(delegate_generic_class *object, _P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4)
    {
    	delegate_mfp *_this = reinterpret_cast<delegate_mfp *>(object);
    	typedef _ReturnType (_FunctionClass::*mfptype)(_P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4);
    	mfptype &mfp = *reinterpret_cast<mfptype *>(&_this->m_rawdata);
    	return (reinterpret_cast<_FunctionClass *>(_this->m_realobject)->*mfp)(p1, p2, p3, p4);
    }

	template<class _FunctionClass, typename _ReturnType, typename _P1Type, typename _P2Type, typename _P3Type, typename _P4Type, typename _P5Type>
    static _ReturnType method_stub(delegate_generic_class *object, _P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4, _P5Type p5)
    {
    	delegate_mfp *_this = reinterpret_cast<delegate_mfp *>(object);
    	typedef _ReturnType (_FunctionClass::*mfptype)(_P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4, _P5Type p5);
    	mfptype &mfp = *reinterpret_cast<mfptype *>(&_this->m_rawdata);
    	return (reinterpret_cast<_FunctionClass *>(_this->m_realobject)->*mfp)(p1, p2, p3, p4, p5);
    }
    
	// helper to convert a function of a given type to a generic function, forcing template
	// instantiation to match the source type
	template <typename _SourceType>
	delegate_generic_function make_generic(_SourceType funcptr)
	{
		return reinterpret_cast<delegate_generic_function>(funcptr);
	}

	// for MSVC maximum size is one pointer, plus 3 ints; all other implementations seem to be smaller
	struct raw_mfp_data
	{
		int data[((sizeof(void *) + 3 * sizeof(int)) + (sizeof(int) - 1)) / sizeof(int)];
		bool operator==(const raw_mfp_data &rhs) const { return (memcmp(data, rhs.data, sizeof(data)) == 0); }
	};

	// internal state
	raw_mfp_data		 		m_rawdata;			// raw buffer to hold the copy of the function pointer
	delegate_generic_class *	m_realobject;		// pointer to the object used for calling
	delegate_generic_function	m_stubfunction;		// pointer to our matching stub function
	static raw_mfp_data			s_null_mfp;			// NULL mfp
};

#elif (USE_DELEGATE_TYPE == DELEGATE_TYPE_INTERNAL)

// ======================> delegate_mfp

// struct describing the contents of a member function pointer
class delegate_mfp
{
public:
	// default constructor
	delegate_mfp()
		: m_function(0),
		  m_this_delta(0) { }

	// copy constructor
	delegate_mfp(const delegate_mfp &src)
		: m_function(src.m_function),
		  m_this_delta(src.m_this_delta) { }

	// construct from any member function pointer
	template<typename _MemberFunctionType, class _MemberFunctionClass, typename _ReturnType, typename _StaticFunctionType>
	delegate_mfp(_MemberFunctionType mfp, _MemberFunctionClass *, _ReturnType *, _StaticFunctionType)
	{
		assert(sizeof(mfp) == sizeof(*this));
		*reinterpret_cast<_MemberFunctionType *>(this) = mfp;
	}

	// comparison helpers
	bool operator==(const delegate_mfp &rhs) const { return (m_function == rhs.m_function && m_this_delta == rhs.m_this_delta); }
	bool isnull() const { return (m_function == 0); }

	// getters
	delegate_generic_class *real_object(delegate_generic_class *original) const { return original; }
	
	// binding helper
	template<typename _FunctionType>
	void update_after_bind(_FunctionType &funcptr, delegate_generic_class *&object)
	{
		funcptr = reinterpret_cast<_FunctionType>(convert_to_generic(object));
	}

private:
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

// ======================> delegate_base

// general delegate class template supporting up to 5 parameters
template<typename _ReturnType, typename _P1Type = _noparam, typename _P2Type = _noparam, typename _P3Type = _noparam, typename _P4Type = _noparam, typename _P5Type = _noparam>
class delegate_base
{
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
		  m_object(NULL),
		  m_name(NULL),
		  m_latebinder(NULL),
		  m_raw_function(NULL) { }

	// copy constructor
	delegate_base(const delegate_base &src)
		: m_function(src.m_function),
		  m_object(NULL),
		  m_name(src.m_name),
		  m_latebinder(src.m_latebinder),
		  m_raw_function(src.m_raw_function),
		  m_raw_mfp(src.m_raw_mfp)
	{
		bind(src.object());
	}

	// copy constructor with late bind
	delegate_base(const delegate_base &src, delegate_late_bind &object)
		: m_function(src.m_function),
		  m_object(NULL),
		  m_name(src.m_name),
		  m_latebinder(src.m_latebinder),
		  m_raw_function(src.m_raw_function),
		  m_raw_mfp(src.m_raw_mfp)
	{
		late_bind(object);
	}

	// construct from member function with object pointer
	template<class _FunctionClass>
	delegate_base(typename traits<_FunctionClass>::member_func_type funcptr, const char *name, _FunctionClass *object)
		: m_function(NULL),
		  m_object(NULL),
		  m_name(name),
		  m_latebinder(&late_bind_helper<_FunctionClass>),
		  m_raw_function(NULL),
		  m_raw_mfp(funcptr, object, (_ReturnType *)0, (generic_static_func)0)
	{
		bind(reinterpret_cast<delegate_generic_class *>(object));
	}

	// construct from static function with object pointer
	template<class _FunctionClass>
	delegate_base(typename traits<_FunctionClass>::static_func_type funcptr, const char *name, _FunctionClass *object)
		: m_function(reinterpret_cast<generic_static_func>(funcptr)),
		  m_object(NULL),
		  m_name(name),
		  m_latebinder(&late_bind_helper<_FunctionClass>),
		  m_raw_function(reinterpret_cast<generic_static_func>(funcptr))
	{
		bind(reinterpret_cast<delegate_generic_class *>(object));
	}

	// construct from static reference function with object reference
	template<class _FunctionClass>
	delegate_base(typename traits<_FunctionClass>::static_ref_func_type funcptr, const char *name, _FunctionClass *object)
		: m_function(reinterpret_cast<generic_static_func>(funcptr)),
		  m_object(NULL),
		  m_name(name),
		  m_latebinder(&late_bind_helper<_FunctionClass>),
		  m_raw_function(reinterpret_cast<generic_static_func>(funcptr))
	{
		bind(reinterpret_cast<delegate_generic_class *>(object));
	}

	// copy operator
	delegate_base &operator=(const delegate_base &src)
	{
		if (this != &src)
		{
			m_function = src.m_function;
			m_object = NULL;
			m_name = src.m_name;
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
		return (m_raw_function == rhs.m_raw_function && object() == rhs.object() && m_raw_mfp == rhs.m_raw_mfp);
	}

	// call the function
	_ReturnType operator()() const { return (*m_function)(m_object); }
	_ReturnType operator()(_P1Type p1) const { return (*m_function)(m_object, p1); }
	_ReturnType operator()(_P1Type p1, _P2Type p2) const { return (*m_function)(m_object, p1, p2); }
	_ReturnType operator()(_P1Type p1, _P2Type p2, _P3Type p3) const { return (*m_function)(m_object, p1, p2, p3); }
	_ReturnType operator()(_P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4) const { return (*m_function)(m_object, p1, p2, p3, p4); }
	_ReturnType operator()(_P1Type p1, _P2Type p2, _P3Type p3, _P4Type p4, _P5Type p5) const { return (*m_function)(m_object, p1, p2, p3, p4, p5); }

	// getters
	bool has_object() const { return (object() != NULL); }
	const char *name() const { return m_name; }
	
	// helpers
	bool isnull() const { return (m_raw_function == NULL && m_raw_mfp.isnull()); }
	bool is_mfp() const { return !m_raw_mfp.isnull(); }

	// late binding
	void late_bind(delegate_late_bind &object) { bind((*m_latebinder)(object)); }

protected:
	// return the actual object (not the one we use for calling)
	delegate_generic_class *object() const { return is_mfp() ? m_raw_mfp.real_object(m_object) : m_object; }

	// late binding function
	typedef delegate_generic_class *(*late_bind_func)(delegate_late_bind &object);

	// late binding helper
	template<class _FunctionClass>
	static delegate_generic_class *late_bind_helper(delegate_late_bind &object)
	{
		_FunctionClass *result = dynamic_cast<_FunctionClass *>(&object);
		if (result == NULL)
			throw binding_type_exception(typeid(_FunctionClass), typeid(object));
		return reinterpret_cast<delegate_generic_class *>(result);
	}

	// bind the actual object
	void bind(delegate_generic_class *object)
	{
		m_object = object;
		
		// if we're wrapping a member function pointer, handle special stuff
		if (m_object != NULL && is_mfp())
			m_raw_mfp.update_after_bind(m_function, m_object);
	}

	// internal state
	generic_static_func			m_function;			// resolved static function pointer
	delegate_generic_class *	m_object;			// resolved object to the post-cast object
	const char *				m_name;				// name string
	late_bind_func				m_latebinder;		// late binding helper
	generic_static_func			m_raw_function;		// raw static function pointer
	delegate_mfp				m_raw_mfp;			// raw member function pointer
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
