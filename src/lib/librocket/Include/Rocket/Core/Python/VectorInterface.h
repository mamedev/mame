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

#ifndef ROCKETCOREPYTHONVECTORINTERFACE_H
#define ROCKETCOREPYTHONVECTORINTERFACE_H

namespace Rocket {
namespace Core {
namespace Python {

/**
	Provides a list style python interface to any STL container
	that can provide a vector style interface.

	Simply contruct one of these objects to register the type
	with python.
	
	VectorInterface< std::vector< std::string > >("StringList");

	@author Lloyd Weehuizen
 */
template <typename Container>
class VectorInterface
{
public:
	VectorInterface(const char* name)
	{
		python::class_< Container >(name)
			.def("__len__", &VectorInterface< Container >::Size)
			.def("__setitem__", &VectorInterface< Container >::SetItem)
			.def("__delitem__", &VectorInterface< Container >::DelItem)
			.def("__getitem__", &VectorInterface< Container >::GetItem, python::return_value_policy< python::return_by_value >())
			.def("__contains__", &VectorInterface< Container >::Contains)			
		;
	}

	static size_t Size(const Container& container)
	{
		return container.size();
	}

	static void SetItem(Container& container, int index, const typename Container::value_type& value)
	{
		if (index < 0)
			index = container.size() + index;
		if (index >= (int)container.size())
			container.resize(index + 1);

		container[index] = value;
	}

	static void DelItem(Container& container, int index)
	{
		if (index >= (int)container.size())
		{
			PyErr_SetString(PyExc_IndexError, String(32, "Invalid index %d", index).CString());
			python::throw_error_already_set();
		}

		container.erase(container.begin() + index);
	}

	static typename Container::value_type& GetItem(Container& container, int index)
	{
		if (index < 0)
			index = container.size() + index;

		if (index >= (int)container.size())
		{
			PyErr_SetString(PyExc_IndexError, String(32, "Invalid index %d", index).CString());
			python::throw_error_already_set();
		}

		return container[index];
	}

	static bool Contains(Container& container, const typename Container::value_type& value)
	{
		for (typename Container::iterator itr = container.begin(); itr != container.end(); ++itr)
		{
			if ((*itr) == value)
				return true;
		}

		return false;
	}
};

}
}
}

#endif

