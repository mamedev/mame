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

#include <Rocket/Core/Variant.h>
#include <Rocket/Core/Dictionary.h>
#include <Rocket/Core/Python/ConverterScriptObject.h>
#include <Rocket/Core/ElementDocument.h>

#include "EventListener.h"

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

namespace Rocket {
namespace Core {
namespace Python {

// Boost helper class for converting from Variant to a python object
struct VariantConverter
{
	VariantConverter()
	{
		// Register custom Variant to python converter
		boost::python::to_python_converter< Variant, VariantConverter >();
		boost::python::to_python_converter< Variant*, VariantConverter >();

		// Register the python to variant converter
		boost::python::converter::registry::push_back(&Convertible, &Construct, boost::python::type_id< Variant >());
	}

	static PyObject* convert(Variant* variant)
	{
		if (!variant)
		{
			Py_INCREF(Py_None);
			return Py_None;
		}
		return convert(*variant);
	}

	static PyObject* convert(const Variant& variant)
	{
		PyObject* object = NULL;

		switch (variant.GetType())
		{
			case Variant::STRING:
			{
				object = PyString_FromString(variant.Get< String >().CString());
			}
			break;

			case Variant::INT:
			{
				object = PyInt_FromLong(variant.Get< int >());
			}
			break;

			case Variant::WORD:
			{
				object = PyInt_FromLong(variant.Get< word >());
			}
			break;

			case Variant::FLOAT:
			{
				object = PyFloat_FromDouble(variant.Get< float >());
			}
			break;

			case Variant::VECTOR2:
			{
				python::object o(variant.Get< Vector2f >());
				object = o.ptr();
				Py_INCREF( object );
			}
			break;

			case Variant::SCRIPTINTERFACE:
			{				
				object = (PyObject*)(variant.Get< ScriptInterface* >())->GetScriptObject();
				Py_INCREF(object);
			}
			break;

			default:
			{
				object = Py_None;
				Py_INCREF(object);
			}
			break;
		}
		
		return object;
	}

	static void* Convertible(PyObject* object)
	{
		if (!PyString_Check(object) && !PyInt_Check(object) && !PyFloat_Check(object))
			return 0;
		return object;
	}

	static void Construct(PyObject* object, boost::python::converter::rvalue_from_python_stage1_data* data)
	{
		void* storage = ((boost::python::converter::rvalue_from_python_storage< Variant >*)data)->storage.bytes;
		Variant* variant = new (storage) Variant();

		if (PyString_Check(object))
		{
			variant->Set(String(PyString_AsString(object)));
		}
		else if (PyInt_Check(object))
		{
			variant->Set((int)PyInt_AsLong(object));
		}
		else if (PyFloat_Check(object))
		{
			variant->Set((float)PyFloat_AsDouble(object));
		}
		else 
		{
			boost::python::throw_error_already_set();
			return;
		}		
		
		data->convertible = storage;
	}
};

// String Converter
struct StringConverter
{
	StringConverter()
	{
		boost::python::to_python_converter< String, StringConverter >();

		boost::python::converter::registry::push_back( &Convertible, &Construct, boost::python::type_id< String >() );
	}

	static PyObject* convert(const String& s)
	{
		return boost::python::incref(boost::python::object(s.CString()).ptr());
	}

	static void* Convertible(PyObject* obj_ptr)
	{
		if (!PyString_Check(obj_ptr))
			return 0;
		return obj_ptr;
	}

	static void Construct(PyObject* obj_ptr, boost::python::converter::rvalue_from_python_stage1_data* data)
	{
		const char* value = PyString_AsString(obj_ptr);
		if (value == 0) 
			boost::python::throw_error_already_set();
		void* storage = ((boost::python::converter::rvalue_from_python_storage< String >*)data)->storage.bytes;
		new (storage) String(value);
		data->convertible = storage;
	}
};

// Helper class for converting from python to an event listener
struct EventListenerFromPython
{
	EventListenerFromPython()
	{
		python::converter::registry::insert( &Convert, python::type_id<EventListener>() );
	}

	static void* Convert( PyObject* object )
	{
		return new EventListener( object );	
	}
};

void RegisterPythonConverters()
{
	StringConverter();
	VariantConverter();	

	ConverterScriptObject< ScriptInterface >();	

	EventListenerFromPython();

	ConverterScriptObject< Context >();
	ConverterScriptObject< Element >();
	ConverterScriptObject< ElementDocument >();
}

}
}
}
