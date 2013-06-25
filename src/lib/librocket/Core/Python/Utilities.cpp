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

#include "precompiled.h"
#include <Rocket/Core/Python/Utilities.h>

namespace Rocket {
namespace Core {
namespace Python {

bool Utilities::IsCallable(PyObject* self, const char* method_name)
{
	PyObject* object = NULL;

	// If its a dictionary, access it like a dictionary
	if (PyDict_Check(self))
	{
		object = PyDict_GetItemString(self, (char*)method_name);
		// This is a borrowed ref, Incref so the decref below is cancelled out		
		Py_XINCREF( object );
	}
	// otherwise attempt normal object access
	else
	{
		object = PyObject_GetAttrString(self, (char*)method_name);
	}

	// If we couldn't find it, clear error and return false
	if (!object)
	{
		PyErr_Clear();
		return false;
	}

	int callable = PyCallable_Check(object);
	
	Py_DECREF(object);
	return callable == 1;
}

bool Utilities::Call(PyObject* object, const char* method_name, const python::tuple& args)
{
	PyObject* result = PythonCall(object, method_name, args);
	if (result)
		Py_DECREF(result);
	return result != NULL;
}

// Print the pending python error to stderr, optionally clearing it
void Utilities::PrintError(bool clear_error)
{
	// Print the error and restore it to the caller
	PyObject *type, *value, *traceback;
	PyErr_Fetch(&type, &value, &traceback);
	Py_XINCREF(type);
	Py_XINCREF(value);
	Py_XINCREF(traceback);
	PyErr_Restore(type, value, traceback);	
	PyErr_Print();
	if (clear_error)
	{
		Py_XDECREF(type);
		Py_XDECREF(value);
		Py_XDECREF(traceback);
	}
	else
	{
		PyErr_Restore(type, value, traceback);		
	}
}

// Converts a python object into an Rocket variant.
bool Utilities::ConvertToVariant(Variant& variant, PyObject* object)
{
	if (PyFloat_Check(object))
	{
		variant.Set((float) PyFloat_AsDouble(object));
		return true;
	}
	else if (PyInt_Check(object))
	{
		variant.Set((int) PyInt_AsLong(object));
		return true;
	}
	else if (PyString_Check(object))
	{
		variant.Set(String(PyString_AsString(object)));
		return true;
	}

	return false;
}

// Internal Python call method
PyObject* Utilities::PythonCall(PyObject* object, const char* method_name, const python::tuple& args )
{
	ROCKET_ASSERT(IsCallable(object, method_name));

	PyObject* result = NULL;
	PyObject* method = NULL;

	if (PyDict_Check(object))
	{
		method = PyDict_GetItemString(object, (char*)method_name);
		// Borrowed reference, so increase so dec at bottom matches
		Py_XINCREF(method);
	}
	else
	{
		method = PyObject_GetAttrString(object, (char*)method_name);
	}

	if (!method)
		return NULL;

	result = PyObject_CallObject(method, args.ptr());
	Py_DECREF(method);
	if (!result)
	{
		PrintError();
		return NULL;		
	}

	return result;
}

}
}
}
