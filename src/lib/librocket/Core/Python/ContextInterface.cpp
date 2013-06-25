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
#include "ContextInterface.h"

#include <Rocket/Core/Python/Utilities.h>
#include <Rocket/Core/Python/Wrapper.h>
#include <Rocket/Core/Context.h>
#include <Rocket/Core/Factory.h>

#include "EventListener.h"
#include "ContextInstancer.h"

namespace Rocket {
namespace Core {
namespace Python {

static PyObject* py_context = NULL;

// Initialises the Python interface.
bool ContextInterface::InitialisePythonInterface()
{
	ContextDocumentProxy::InitialisePythonInterface();

	py_context = python::class_< Context, Rocket::Core::Python::Wrapper< Context, const Rocket::Core::String& >, boost::noncopyable>("Context", python::init< const Rocket::Core::String& >())
		.def("AddEventListener", &ContextInterface::AddEventListener)
		.def("AddMouseCursor", &Context::AddMouseCursor, python::return_value_policy< python::return_by_value >())
		.def("CreateDocument", &ContextInterface::CreateDocument)
		.def("LoadDocument", &ContextInterface::LoadDocument)
		.def("LoadDocumentFromMemory", &ContextInterface::LoadDocumentFromMemory)
		.def("LoadMouseCursor", &ContextInterface::LoadMouseCursor)
		.def("Render", &Context::Render)
		.def("ShowMouseCursor", &Context::ShowMouseCursor)
		.def("UnloadAllDocuments", &Context::UnloadAllDocuments)
		.def("UnloadAllMouseCursors", &Context::UnloadAllMouseCursors)
		.def("UnloadDocument", &Context::UnloadDocument)
		.def("UnloadMouseCursor", &Context::UnloadMouseCursor)
		.def("Update", &Context::Update)
		.add_property("dimensions", python::make_function(&Context::GetDimensions, python::return_value_policy< python::return_by_value >()), &Context::SetDimensions)
		.add_property("documents", &ContextInterface::GetDocuments)
		.add_property("focus_element", python::make_function(&Context::GetFocusElement, python::return_value_policy< python::return_by_value >()))
		.add_property("hover_element", python::make_function(&Context::GetHoverElement, python::return_value_policy< python::return_by_value >()))
		.add_property("root_element", python::make_function(&Context::GetRootElement, python::return_value_policy< python::return_by_value >()))
		.add_property("name", python::make_function(&Context::GetName, python::return_value_policy< python::return_by_value >()))
		.ptr();

	return true;
}

// Initialise the Rocket element interface.
void ContextInterface::InitialiseRocketInterface()
{
	Factory::RegisterContextInstancer(new ContextInstancer(py_context))->RemoveReference();
}

// The "AddEventListener" function bound into Python context objects instead of the C++ function.
void ContextInterface::AddEventListener(Context* self, const char* event, const char* script, bool in_capture_phase)
{
	self->AddEventListener(event, new EventListener(script, self->GetRootElement()), in_capture_phase);
}

// The "CreateDocument" function bound into Python context objects instead of the C++ function.
python::object ContextInterface::CreateDocument(Context* self, const char* tag)
{
	Rocket::Core::ElementDocument* document = self->CreateDocument(tag);
	if (document == NULL)
		return python::object();

	// Remove the C++ caller reference and add a Python one to replace it.
	python::object py_document = Rocket::Core::Python::Utilities::MakeObject(document);
	document->RemoveReference();

	return py_document;
}

// The "LoadDocument" function bound into Python context objects instead of the C++ function.
python::object ContextInterface::LoadDocument(Context* self, const char* document_path)
{
	Rocket::Core::ElementDocument* document = self->LoadDocument(document_path);
	if (document == NULL)
		return python::object();

	// Remove the C++ caller reference and return the python::object
	python::object py_document = Rocket::Core::Python::Utilities::MakeObject(document);
	document->RemoveReference();

	return py_document;
}

// The "LoadDocument" function bound into Python context objects instead of the C++ function.
python::object ContextInterface::LoadDocumentFromMemory(Context* self, const char* stream)
{
	Rocket::Core::ElementDocument* document = self->LoadDocumentFromMemory(stream);
	if (document == NULL)
		return python::object();

	// Remove the C++ caller reference and return the python::object
	python::object py_document = Rocket::Core::Python::Utilities::MakeObject(document);
	document->RemoveReference();

	return py_document;
}

// The "LoadMouseCursor" function bound into Python context objects instead of the C++ function.
python::object ContextInterface::LoadMouseCursor(Context* self, const char* document_path)
{
	Rocket::Core::ElementDocument* document = self->LoadMouseCursor(document_path);
	if (document == NULL)
		return python::object();

	// Remove the C++ caller reference and add a Python one to replace it.
	python::object py_document = Rocket::Core::Python::Utilities::MakeObject(document);
	document->RemoveReference();

	return py_document;
}

// Returns the document proxy object for the 'document' property.
ContextDocumentProxy ContextInterface::GetDocuments(Context* self)
{
	return ContextDocumentProxy(self);
}

}
}
}
