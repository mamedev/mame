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

#ifndef ROCKETCOREPYTHONELEMENTINSTANCER_H
#define ROCKETCOREPYTHONELEMENTINSTANCER_H

#include <Rocket/Core/String.h>
#include <Rocket/Core/XMLParser.h>
#include <Rocket/Core/Python/Python.h>
#include <Rocket/Core/Element.h>
#include <Rocket/Core/ElementInstancer.h>
#include <Rocket/Core/Python/Header.h>

namespace Rocket {
namespace Core {
namespace Python {

/**
	Python Element Instancer 

	This object provides the standard ElementInstancer 
	interface for instancing python elements.

	NOTE: This is all inline so that it can be used by 3rdparty plugins

	@author Lloyd Weehuizen 
 */

class ElementInstancer : public Rocket::Core::ElementInstancer
{
public:
	ElementInstancer(PyObject* _class_definition)
	{
		class_definition = _class_definition;
		Py_INCREF(class_definition);
	}
	virtual ~ElementInstancer()
	{
		Py_DECREF(class_definition);
	}

	/// Instances an element given the tag name and attributes
	/// @param tag Name of the element to instance
	/// @param attributes vector of name value pairs
	virtual Element* InstanceElement(Element* ROCKET_UNUSED(parent), const Rocket::Core::String& tag, const Rocket::Core::XMLAttributes& ROCKET_UNUSED(attributes))
	{
		// Build the arguments
		PyObject* args = PyTuple_New(1);
		PyTuple_SetItem(args, 0, PyString_FromString(tag.CString()));

		PyObject* instance = PyObject_CallObject(class_definition, args);
		Py_DECREF(args);

		// Check the instance was created successfully
		if (instance == NULL)
		{
			PyErr_Print();
			return NULL;
		}

		Element* element = python::extract<Element*>(python::object(python::handle<>(python::borrowed(instance))));	

		// Need to switch the Python reference count to a true C++ reference count.
		element->AddReference(); // Adds to C++ and Python reference counts.
		Py_DECREF( instance );   // Removes from Python.

		return element;
	}

	/// Releases the given element
	/// @param element to release
	virtual void ReleaseElement(Element* ROCKET_UNUSED(element))
	{
		// Never release Python elements, Python will manage this for us.
	}

	/// Release the instancer
	virtual void Release()
	{
		delete this;
	}

private:
	PyObject* class_definition;
};

}
}
}

#endif
