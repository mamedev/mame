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

#ifndef ROCKETCOREPICKLETYPECONVERTER_H
#define ROCKETCOREPICKLETYPECONVERTER_H

#include <Rocket/Core/TypeConverter.h>

namespace Rocket {
namespace Core {
namespace Python {

/**
	Generic Python Pickler that does string conversion using an Rocket::Core::TypeConverter.

	@author Lloyd Weehuizen
 */

template <typename T>
class PickleTypeConverter : public python::pickle_suite
{
public:	
	static python::tuple getstate(python::object obj)
	{
		T& object = python::extract<T&>(obj)();
		
		String buffer;
		TypeConverter< T, String >::Convert(object, buffer);

		return python::make_tuple(buffer.CString());
	}

	static void setstate(python::object obj, python::tuple state)
	{
		T& object = python::extract< T& >(obj)();	

		int len = python::extract< int >(state.attr( "__len__" )());
		if (len != 1)
		{
			PyErr_SetObject(PyExc_ValueError,
							("expected 1-item tuple in call to __setstate__; got %s"
							% state).ptr()
				);
			python::throw_error_already_set();
		}
		
		const char* cstring = python::extract<const char*>(state[0]);
		String string(cstring);
		TypeConverter<String, T>::Convert(string, object);
	}

	static bool getstate_manages_dict()
	{
		// Tell boost we've taken care of the dictionary
		return true; 
	}
};

}
}
}

#endif
