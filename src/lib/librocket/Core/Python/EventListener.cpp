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
#include "EventListener.h"

#include <Rocket/Core/Python/Utilities.h>

#include "EventWrapper.h"
#include "ElementDocumentWrapper.h"

namespace Rocket {
namespace Core {
namespace Python {

EventListener::EventListener(PyObject* object)
{
	callable = NULL;
	element = NULL;
	global_namespace = NULL;

	if (PyCallable_Check(object))
	{
		// Its a callable, store it
		callable = object;
		Py_INCREF(callable);

		// Now look up the global namespace for the function/method
		PyObject* func_globals = NULL;

		// Methods have an im_func attribute that contains the function details
		PyObject* im_func = PyObject_GetAttrString(callable, "im_func");
		if (!im_func)
		{
			PyErr_Clear();
			// Functions just have a func_globals directly
			func_globals = PyObject_GetAttrString(callable, "func_globals");
		}
		else
		{
			func_globals = PyObject_GetAttrString(im_func, "func_globals");
			Py_DECREF(im_func);
		}

		if (func_globals)
		{		
			// Keep a reference to func_globals, global_namespace tracks a reference
			global_namespace = func_globals;
		}
		else
		{
			// Otherwise clear the error, we'll resort to using the document namespace
			PyErr_Clear();
		}
	}
	else if (PyString_Check(object))
	{
		// Its a string, we need to compile it
		source_code = PyString_AsString(object);
	}
	else
	{
		// Unknown, log an error
		Log::Message(Rocket::Core::Log::LT_ERROR, "Failed to initialise python based event listener. Unknown python object type, should be a callable or a string."); 
	}
}

EventListener::EventListener(const Rocket::Core::String& code, Element* context)
{
	callable = NULL;
	element = context;
	global_namespace = NULL;

	// Load the code for this listener
	source_code = code;
}

EventListener::~EventListener()
{
	if (callable)
		Py_DECREF(callable);
	if (global_namespace)
		Py_DECREF(global_namespace);	
}

void EventListener::ProcessEvent(Event& event)
{
	// If we've got nothing to do, abort
	if (!callable && source_code.Empty())
		return;

	// Do we need to compile the code?
	if (!callable && !source_code.Empty())
	{
		// Attempt to compile the code
		if (!Compile())
			return;
	}

	PyObject* py_namespace = GetGlobalNamespace();

	// Store current globals
	PyObject* old_event = PyDict_GetItemString(py_namespace, "event");
	PyObject* old_self = PyDict_GetItemString(py_namespace, "self");
	PyObject* old_document = PyDict_GetItemString(py_namespace, "document");
	// Clear any pending errors (KeyErrors if they didn't exist)
	PyErr_Clear();

	// Increase the ref to store the old values locally
	Py_XINCREF(old_event);
	Py_XINCREF(old_self);
	Py_XINCREF(old_document);

	// Set up the new expected globals
	PyDict_SetItemString(py_namespace, "event", Rocket::Core::Python::Utilities::MakeObject(&event).ptr());
	PyDict_SetItemString(py_namespace, "self", Rocket::Core::Python::Utilities::MakeObject(element).ptr());
	PyDict_SetItemString(py_namespace, "document", Rocket::Core::Python::Utilities::MakeObject(element->GetOwnerDocument()).ptr());

	// Call the bound function
	PyObject* result = NULL;
	try
	{
		result = PyObject_CallObject(callable, NULL);
	}
	catch (python::error_already_set&)
	{		
	}

	// Check error conditions
	if (result)
	{
		Py_DECREF(result);
	}
	else
	{		
		Rocket::Core::Python::Utilities::PrintError(true);
	}

	// Remove the globals
	PyDict_DelItemString(py_namespace, "document");
	PyDict_DelItemString(py_namespace, "self");
	PyDict_DelItemString(py_namespace, "event");

	// Restore old events if necessary
	if (old_event)
	{
		PyDict_SetItemString(py_namespace, "event", old_event);
		Py_DECREF(old_event);
	}
	if (old_self)
	{
		PyDict_SetItemString(py_namespace, "self", old_self);
		Py_DECREF(old_self);
	}
	if (old_document)
	{
		PyDict_SetItemString(py_namespace, "document", old_document);
		Py_DECREF(old_document);
	}
}

void EventListener::OnAttach(Element* _element)
{
	if (!element)
		element = _element;
}

void EventListener::OnDetach(Element* /*element*/)
{
	delete this;
}

bool EventListener::Compile()
{
	Rocket::Core::String function_name(64, "Event_%x", this);
	Rocket::Core::String function_code(64, "def %s():", function_name.CString());

	Rocket::Core::StringList lines;
	Rocket::Core::StringUtilities::ExpandString(lines, source_code, ';');
	for (size_t i = 0; i < lines.size(); i++)
	{
		// Python doesn't handle \r's, strip em and indent the code correctly
		function_code += Rocket::Core::String(1024, "\n\t%s", lines[i].CString()).Replace("\r", "");
	}

	ROCKET_ASSERT(element != NULL);

	PyObject* py_namespace = GetGlobalNamespace();

	// Add our function to the namespace
	PyObject* result = PyRun_String(function_code.CString(), Py_file_input, py_namespace, py_namespace);
	if (!result)
	{
		Rocket::Core::Python::Utilities::PrintError();		
		return false;
	}
	Py_DECREF(result);

	// Get a handle to our function
	callable = PyDict_GetItemString(py_namespace, function_name.CString());
	Py_INCREF(callable);

	return true;
}

PyObject* EventListener::GetGlobalNamespace()
{
	if (global_namespace)
	{
		return global_namespace;
	}

	ElementDocumentWrapper* document = dynamic_cast< ElementDocumentWrapper* >(element->GetOwnerDocument());
	if (!document)
	{
		Log::Message(Rocket::Core::Log::LT_ERROR, "Failed to find python accessible document for element %s, unable to create event. Did you create a context/document before initialising RocketPython?", element->GetAddress().CString());
		return NULL;
	}

	global_namespace = document->GetModuleNamespace();
	Py_INCREF(global_namespace);
	return global_namespace;
}

}
}
}
