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

#ifndef ROCKETCOREPYTHONNAMEINDEXINTERFACE_H
#define ROCKETCOREPYTHONNAMEINDEXINTERFACE_H

#include <Rocket/Core/Python/Utilities.h>

namespace Rocket {
namespace Core {
namespace Python {

template < typename HostType >
class NameAccessorInvalid
{
public:
	python::object operator()(HostType& host, const char* name)
	{
		PyErr_SetString(PyExc_KeyError, "Invalid key.");
		python::throw_error_already_set();

		return python::object();
	}
};

/**
	Template class for generating a lightweight proxy object for providing array-style lookups on a C++ object.

	@author Peter Curry
 */

template < typename HostType, typename LengthAccessor, typename IndexAccessor, typename NameAccessor = NameAccessorInvalid< HostType > >
class NameIndexInterface : public python::def_visitor< NameIndexInterface< HostType, LengthAccessor, IndexAccessor, NameAccessor > >
{
public:
	NameIndexInterface()
	{
	}

	~NameIndexInterface()
	{
	}

	/// Initialise the interface.
	template < typename ClassType >
	void visit(ClassType& _class) const
	{
		_class.def("__getitem__", &NameIndexInterface< HostType, LengthAccessor, IndexAccessor, NameAccessor >::GetItem);
		_class.def("__len__", &NameIndexInterface< HostType, LengthAccessor, IndexAccessor, NameAccessor >::Len);
	}

	/// Python __getitem__ override.
	static python::object GetItem(HostType& host, python::object key)
	{
		static NameAccessor name_accessor;
		static LengthAccessor length_accessor;
		static IndexAccessor index_accessor;

		if (PyString_Check(key.ptr()))
		{
			return Rocket::Core::Python::Utilities::MakeObject(name_accessor(host, PyString_AsString(key.ptr())));
		}
		else if (PyInt_Check(key.ptr()))
		{
			int index = PyInt_AsLong(key.ptr());

			// Support indexing from both ends.
			if (index < 0)
				index = length_accessor(host) + index;

			// Throw exception if we're out of range, this is required to support python iteration.
			if (index >= length_accessor(host))
			{
				PyErr_SetString(PyExc_IndexError, "Index out of range.");
				python::throw_error_already_set();
			}

			return Rocket::Core::Python::Utilities::MakeObject(index_accessor(host, index));
		}

		PyErr_SetString(PyExc_KeyError, "Invalid key.");
		python::throw_error_already_set();

		return python::object();
	}

	/// Python __len__ override.
	static int Len(HostType& host)
	{
		static LengthAccessor length_accessor;
		return length_accessor(host);
	}
};

}
}
}

#endif
