/*
 * This source file is part of libRocket, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://www.librocket.com
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#if !defined(BOOST_PP_IS_ITERATING)
#  error PyWrapperIter - do not include this file!
#endif

#define N BOOST_PP_ITERATION()

/**
	Generic Python Wrapper, using Boost Preprocessor Iteration to Generate Variants
	
	Relies on a Rocket::Core::ReferenceCountable base class for reference counting.
	
	Traps ReferenceActivated/Deactivated calls and makes them behave like
	they should for python based objects.
	
	@author Lloyd Weehuizen
 */

#define WRAPPER_PARAM(x, n, d) , d

template < typename T BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_PARAMS_Z(1, N, typename A) >
class Wrapper< T BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_PARAMS_Z(1, N, A) BOOST_PP_REPEAT_1( BOOST_PP_SUB(BOOST_PP_INC(WRAPPER_MAX_ARGS), N), WRAPPER_PARAM, WrapperNone) > : public T
{
public:	
	Wrapper(PyObject* self BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_BINARY_PARAMS_Z(1, N, A, a)) : T(BOOST_PP_ENUM_PARAMS_Z(1, N, a))
	{
		// Set self to NULL, so we can trap the reference deactivated and not pass it down
		this->self = NULL;

		// We have to remove the C++ reference count that all C++ objects start with here,
		// otherwise if an object is created in python and destroyed by python, the C++ ref count will
		// remain 1. The PyWrapperInstancer will increase the refcount again, to ensure a correct refcount
		// if the object was created by C++
		T::RemoveReference();

		// If the C++ reference count is not 0 at this point, it means additional references have been added
		// during the classes constructor. We have to propogate these references into python.
		for (int i = 0; i < this->T::GetReferenceCount(); i++)
			Py_INCREF(self);

		// Store self
		this->self = self;
	}

	Wrapper(PyObject* self, const T& other) : T(other)
	{
		this->self = self;		
	}

	virtual ~Wrapper()
	{		
		// We should only be deleted when python says the refcnt is 0, if we
		// are being deleted prematurely, something is wrong!
		ROCKET_ASSERTMSG(self->ob_refcnt == 0, "Python object being cleared up prematurely, reference count not 0.");
		ROCKET_ASSERT(this->T::GetReferenceCount() == 0);
	}

	/// Override AddReference so we can push the call through to python
	virtual void AddReference()
	{
		Py_INCREF(self);
		
		T::AddReference();
	}

	/// Overrride RemoveReference so we can push the call through to python
	virtual void RemoveReference()
	{
		T::RemoveReference();

		Py_DECREF(self);
	}
	
	virtual int GetReferenceCount()
	{
		// C++ reference counts are always reflected in the python ref count
		return self->ob_refcnt;
	}

	virtual void OnReferenceDeactivate()
	{
		// If self is NULL, don't pass the call down, as this is the initial 
		// T::RemoveReference from the constructor
		if (self)
			T::OnReferenceDeactivate();
	}

	// Script object access
	virtual void* GetScriptObject() const { return self; }

protected:
	PyObject* self;
};

#undef WRAPPER_PARAM

