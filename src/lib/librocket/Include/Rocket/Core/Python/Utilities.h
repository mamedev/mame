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

#ifndef ROCKETCOREPYTHONUTILITIES_H
#define ROCKETCOREPYTHONUTILITIES_H

#include <Rocket/Core/Variant.h>
#include <Rocket/Core/Python/Header.h>
#include <Rocket/Core/Python/Python.h>

namespace Rocket {
namespace Core {
namespace Python {

class ROCKETCOREPYTHON_API Utilities
{
public:
	/// Is the specific method name callable
	static bool IsCallable(PyObject* self, const char* method_name);

	/// Call a python function
	/// @param[in] object Python Dictionary or Object
	/// @param[in] method_name function or method name to call
	/// @param[in] args Tuple of arguments
	/// @returns The python result
	template <typename R>
	static R Call(PyObject* object, const char* method_name, const python::tuple& args)
	{
		PyObject* result = PythonCall(object, method_name, args);		
		
		python::converter::return_from_python<R> converter;
		return converter(result);		
	}	
	/// Call a python function
	/// @param[in] object Python Dictionary or Object to lookup the method in
	/// @param[in] method_name function or method name to call
	/// @param[in] args Tuple of arguments
	/// @returns True on success
	static bool Call(PyObject* object, const char* method_name, const python::tuple& args);

	/// Creates a boost python object from an existing C++ object pointer
	/// @param[in] object The C++ object to convert
	/// @returns A boost::python::object referencing the same object
	template< typename T >
	static inline python::object MakeObject(T* object)
	{
		// Constructs a boost::object using a result_converter to get a pointer to the existing object
		typename python::return_by_value::apply< T* >::type result_converter;
		return python::object(python::handle<>(result_converter(object)));
	}
	/// Creates a new python object (copy) from a C++ object
	/// @param[in] object The C++ object to convert
	/// @returns A boost::python::object that contains a copy of the object
	template< typename T >
	static inline python::object MakeObject(const T& object)
	{
		// Constructs a boost::object copy of the C++ object
		return python::object(object);		
	}

	/// Print any pending exceptions to stderr
	/// @param[in] clear_error Clear the error flag
	static void PrintError(bool clear_error = false);

	/// Converts a python object into an Rocket variant.
	/// @param[out] variant The variant to convert the object into.
	/// @param[in] object The python object to convert.
	/// @return True if the conversion was successful, false otherwise.
	static bool ConvertToVariant(Variant& variant, PyObject* object);

private:
	static PyObject* PythonCall(PyObject* object, const char* method_name, const python::tuple& args );
};

// Template Specialisation for converting from a PyObject*
template<>
inline python::object Utilities::MakeObject< PyObject >(PyObject* object)
{
	return python::object(python::handle<>(python::borrowed(object)));
}

}
}
}

#endif

