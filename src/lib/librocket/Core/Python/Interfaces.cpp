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
#include <Rocket/Core/URL.h>
#include <Rocket/Core/Log.h>
#include <Rocket/Core/Dictionary.h>
//#include "DataSourceWrapper.h"
#include <Rocket/Core/Python/ConverterScriptObject.h>
#include <Rocket/Core/Python/PickleTypeConverter.h>
#include <Rocket/Core/Python/Utilities.h>
#include <Rocket/Core/Python/VectorInterface.h>
#include <vector>

namespace Rocket {
namespace Core {
namespace Python {

// Dictionary Interface
class DictionaryInterface
{
public:
	DictionaryInterface()
	{
		python::class_< Dictionary >("Dictionary")
			.def("__len__", &DictionaryInterface::Size)
			.def("__setitem__", &DictionaryInterface::SetItem)
			.def("__delitem__", &DictionaryInterface::DelItem)
			.def("__getitem__", &DictionaryInterface::GetItem, python::return_value_policy< python::return_by_value >())
			.def("__contains__", &DictionaryInterface::Contains)			
		;
	}

	static size_t Size(const Dictionary& dict)
	{
		return dict.Size();
	}

	static void SetItem(Dictionary& dict, const char* key, const Variant& value)
	{
		dict.Set(key, value);
	}

	static void DelItem(Dictionary& dict, const char* key)
	{
		dict.Remove(key);
	}

	static Variant& GetItem(const Dictionary& dict, const char* key)
	{
		Variant* variant = dict.Get(key);
		if (!variant)
		{
			PyErr_SetString(PyExc_KeyError, Rocket::Core::String(64, "Invalid key %s", key).CString());
			python::throw_error_already_set();
		}
		return *variant;
	}

	static bool Contains(const Dictionary& dict, const char* key)
	{
		return dict.Get(key) != NULL;
	}
};

static void Log(Log::Type level, const char* message)
{	
	Core::Log::Message(level, "%s", message);
}

void RegisterPythonInterfaces()
{
	python::class_< ScriptInterface, boost::noncopyable >("ScriptInterface", python::no_init);

	VectorInterface< StringList >("StringList");

	python::class_< Vector2f >("Vector2f")
		.def(python::init< float, float >())
		.def_readwrite("x", &Vector2f::x)
		.def_readwrite("y", &Vector2f::y)
		.def(python::self * float())	 // * float
		.def(python::self / float())	 // / float
		.def(python::self + Vector2f())  // + Vector2f
		.def(python::self - Vector2f())  // - Vector2f
		.def(python::self == Vector2f()) // ==
		.def(python::self != Vector2f()) // !=
		.def("DotProduct", &Vector2f::DotProduct)
		.def("Normalise", &Vector2f::Normalise)
		.def("Rotate", &Vector2f::Rotate)
		.add_property("magnitude", &Vector2f::Magnitude)
		.def_pickle(PickleTypeConverter< Vector2f >())
	;

	python::class_< Vector2i >("Vector2i")
		.def(python::init< int, int >())
		.def_readwrite("x", &Vector2i::x)
		.def_readwrite("y", &Vector2i::y)
		.def(python::self * int())		 // * int
		.def(python::self / int())		 // / int
		.def(python::self + Vector2i())	 // + Vector2i
		.def(python::self - Vector2i())	 // - Vector2i
		.def(python::self == Vector2i()) // ==
		.def(python::self != Vector2i()) // !=
		.def_pickle(PickleTypeConverter< Vector2i >())
	;

	python::class_< Colourf >("Colourf")
		.def(python::init< float, float, float, float >())
		.def_readwrite("red", &Colourf::red)
		.def_readwrite("green", &Colourf::green)
		.def_readwrite("blue", &Colourf::blue)
		.def_readwrite("alpha", &Colourf::alpha)
		.def(python::self == Colourf()) // ==
		.def_pickle(PickleTypeConverter< Colourf >())
	;

	python::class_< Colourb >("Colourb")
		.def(python::init< byte, byte, byte, byte>())
		.def_readwrite("red", &Colourb::red)
		.def_readwrite("green", &Colourb::green)
		.def_readwrite("blue", &Colourb::blue)
		.def_readwrite("alpha", &Colourb::alpha)
		.def(python::self == Colourb()) // ==
		.def(python::self + Colourb()) // +
		.def(python::self *= float()) // *=
		.def(python::self * float()) // *
		.def_pickle(PickleTypeConverter< Colourb >())
	;

	python::class_< URL >("URL")
		.def(python::init< const char* >())
		.def_pickle(PickleTypeConverter< URL >())
	;

	python::def("Log", &Log);
	python::enum_< Core::Log::Type >("logtype")
		.value("always", Core::Log::LT_ALWAYS)
		.value("error", Core::Log::LT_ERROR)
		.value("warning", Core::Log::LT_WARNING)
		.value("info", Core::Log::LT_INFO)
		.value("debug", Core::Log::LT_DEBUG)
	;

	DictionaryInterface();
	//DataSourceWrapper::InitialisePythonInterface();
}

}
}
}
