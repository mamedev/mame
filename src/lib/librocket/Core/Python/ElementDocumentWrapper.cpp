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
#include "ElementDocumentWrapper.h"
#include <Rocket/Core/Stream.h>
#include <Rocket/Core/Python/Utilities.h>

namespace Rocket {
namespace Core {
namespace Python {

ElementDocumentWrapper::ElementDocumentWrapper(PyObject* self, const char* tag) : ElementWrapper< ElementDocument >(self, tag)
{
	Rocket::Core::String module_id(32, "document_%x", this);

	// Create a new module
	module = PyModule_New(module_id.CString());
	module_namespace = PyModule_GetDict(module);

	// Merging main into the module
	PyObject* builtins = PyImport_AddModule("__main__");
	PyObject* builtins_dict = PyModule_GetDict( builtins);
	PyDict_Merge(module_namespace, builtins_dict, 0);
}

ElementDocumentWrapper::~ElementDocumentWrapper()
{
	// Release the module
	Py_DECREF(module);
}

void ElementDocumentWrapper::LoadScript(Rocket::Core::Stream* stream, const Rocket::Core::String& source_name)
{	
	// If theres a source, check if the code is already loaded and just reuse it
	if (!source_name.Empty())
	{		
		Rocket::Core::String module_name = Rocket::Core::String(source_name).Replace("/", "_");
		module_name = module_name.Replace("\\", "_");
		module_name = module_name.Replace(".py", "");

		PyObject* modules = PyImport_GetModuleDict();
		PyObject* merge_module = PyDict_GetItemString(modules, module_name.CString());

#ifdef ROCKET_DEBUG
		// In debug builds, force module to NULL so that scripts are always reloaded
		merge_module = NULL;
#else
		if (merge_module)
		{
			Py_INCREF(merge_module);
		}
#endif

		if (!merge_module)
		{
			// Compile the code as a python module
			Rocket::Core::String source_buffer;
			PreprocessCode(source_buffer, stream);		
		
			PyObject* code = Py_CompileString(source_buffer.CString(), source_name.CString(), Py_file_input);
			if (code)
			{
				merge_module = PyImport_ExecCodeModule((char*)module_name.CString(), code);
				Py_DECREF(code);
			}			
		}

		if (merge_module)
		{
			PyObject* dict = PyModule_GetDict(merge_module);
			PyDict_Merge(module_namespace, dict, 0);
			Py_DECREF(merge_module);
		}
		else
		{
			Rocket::Core::Python::Utilities::PrintError();			
		}
	}
	else
	{
		// Compile directly onto the python module
		Rocket::Core::String source_buffer;
		PreprocessCode(source_buffer, stream);
		
		PyObject* result = PyRun_String(source_buffer.CString(), Py_file_input, module_namespace, module_namespace);
		if ( !result )
		{
			Rocket::Core::Python::Utilities::PrintError();
		}
		else
		{
			Py_DECREF(result);
		}	
	}
}

PyObject* ElementDocumentWrapper::GetModuleNamespace()
{
	return module_namespace;
}

void ElementDocumentWrapper::PreprocessCode(Rocket::Core::String &code, Rocket::Core::Stream *stream)
{
	// Load in the script	
	Rocket::Core::String buffer;	
	stream->Read(buffer, stream->Length());	

	// Strip comments and build up code
	code = "";
	
	size_t i = 0;
	size_t line_start = 0;
	enum ParseState { START, COMMENT, DATA };
	ParseState state = START;

	while (i < buffer.Length())
	{		
		// Python doesn't like \r's, strip them
		if (buffer[i] == '\r')
		{
			buffer.Erase(i, 1);

			// Make sure we get out if there are no characters left
			if (i == buffer.Length())
				continue;
		}

		switch (state)
		{
			case START:
			{
				// Check for the start of comments or non whitespace data
				if (buffer[i] == '#')
					state = COMMENT;
				else if (!Rocket::Core::StringUtilities::IsWhitespace(buffer[i]))
					state = DATA;
			}
			break;

			default:		
			{
				// If we've hit the end of the line, process as required
				if (buffer[i] == '\n')
				{				
					if (state == DATA)
						code += buffer.Substring(line_start, i - line_start + 1);

					state = START;
					line_start = i + 1;
				}
			}
			break;
		}
		
		i++;
	}
}

}
}
}
