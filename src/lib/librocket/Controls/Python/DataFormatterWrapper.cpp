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
#include "DataFormatterWrapper.h"

namespace Rocket {
namespace Controls {
namespace Python {

DataFormatterWrapper::DataFormatterWrapper(PyObject* _self, const char* name) : DataFormatter(name)
{
	self = _self;
}

DataFormatterWrapper::~DataFormatterWrapper()
{
}

void DataFormatterWrapper::InitialisePythonInterface()
{
	python::class_<DataFormatter, DataFormatterWrapper, boost::noncopyable>("DataFormatter", python::init<const char*>());
}

void DataFormatterWrapper::FormatData(Rocket::Core::String& formatted_data, const Rocket::Core::StringList& raw_data)
{
	PyObject* callable = PyObject_GetAttrString(self, "FormatData");
	if (!callable)
	{
		PyObject* name = PyObject_GetAttrString(self, "__name__");
		Rocket::Core::Log::Message(Rocket::Core::Log::LT_WARNING, "Function \"FormatData\" not found on python data formatter %s.", PyString_AsString(name));
		Py_DECREF(name);
		return;
	}

	python::tuple t = python::make_tuple(raw_data);
	PyObject* result = PyObject_CallObject(callable, t.ptr());
	Py_DECREF(callable);

	// If it's a string, then just return it.
	if (result && PyString_Check(result))
	{
		formatted_data = PyString_AsString(result);
	}
	else
	{
		Rocket::Core::Log::Message(Rocket::Core::Log::LT_ERROR, "Failed to get valid string from data formatter %s.", GetDataFormatterName().CString());
		if (!result)
		{
			python::throw_error_already_set();
		}
	}

	if (result)
		Py_DECREF(result);
}

}
}
}
