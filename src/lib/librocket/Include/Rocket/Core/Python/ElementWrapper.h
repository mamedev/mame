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

#ifndef ROCKETCOREPYTHONELEMENTWRAPPER_H
#define ROCKETCOREPYTHONELEMENTWRAPPER_H

namespace Rocket {
namespace Core {
namespace Python {

/**
	Python Wrapper class for Elements 
 
	This class maintains is a standard boost wrapper class that 
	the self pointer for an element and provides virtual 
	overrides for the hook methods. 
 
	@author Lloyd Weehuizen 
 */

template <typename T>
class ElementWrapper : public T
{
public:
	ElementWrapper( PyObject* self, const char* tag ) : self( self ), T( tag )
	{
		// Force remove reference, to reduce the count to 0 as python has instanced
		// this object. If the call originally came from C++ (PyBRElementInstancer)
		// then it will add its own reference
		T::RemoveReference();

		// If the C++ reference count is not 0 at this point, it means additional references have been added
		// during the classes constructor. We have to propogate these references into python.
		for (int i = 0; i < this->T::GetReferenceCount(); i++)
			Py_INCREF(self);
	}
	virtual ~ElementWrapper() {}

	/// Return the python script object associated with this element
	virtual void* GetScriptObject() const { return self; }

	// Propogate add ref's into python
	virtual void AddReference()
	{
		Py_INCREF( self );

		T::AddReference();
	}

	// Propogate remove ref's into python
	virtual void RemoveReference()
	{
		T::RemoveReference();

		Py_DECREF( self );
	}

	virtual void OnReferenceDeactivate()
	{
		// Ignore reference deactivated
	}

private:
	// Python representation of this object
	PyObject* self;
};

}
}
}

#endif
