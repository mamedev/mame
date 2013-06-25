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
#include "ElementInterface.h"
#include <Rocket/Core/Python/Utilities.h>
#include <Rocket/Core/Python/VectorInterface.h>
#include "../ElementHandle.h"
#include "../ElementImage.h"
#include "../ElementTextDefault.h"
#include <Rocket/Core/ElementUtilities.h>
#include <Rocket/Core/Factory.h>
#include "ElementAttributeProxy.h"
#include "ElementChildrenProxy.h"
#include "ElementDocumentWrapper.h"
#include "ElementStyleProxy.h"
#include <Rocket/Core/Python/ElementInstancer.h>
#include <Rocket/Core/Python/ElementWrapper.h>

namespace Rocket {
namespace Core {
namespace Python {

typedef std::map< Rocket::Core::String, PyObject* > ClassDefinitions;
ClassDefinitions class_definitions;

void ElementInterface::InitialisePythonInterface()
{
	ElementStyleProxy::InitialisePythonInterface();
	ElementChildrenProxy::InitialisePythonInterface();
	ElementAttributeProxy::InitialisePythonInterface();

	// Element list
	Rocket::Core::Python::VectorInterface< ElementList > element_list("ElementList");

	// Document focus flags
	python::enum_< ElementDocument::FocusFlags >("focus")
		.value("NONE", ElementDocument::NONE)
		.value("FOCUS", ElementDocument::FOCUS)
		.value("MODAL", ElementDocument::MODAL)
		;

	void (*AddEventListener)(Element* element, const char* event, Rocket::Core::Python::EventListener* listener, bool in_capture_phase) = &ElementInterface::AddEventListener;
	void (*AddEventListenerDefault)(Element* element, const char* event, Rocket::Core::Python::EventListener* listener) = &ElementInterface::AddEventListener;

	// Define the basic element type.
	class_definitions["Element"] = python::class_< Element, ElementWrapper< Element >, boost::noncopyable >("Element", python::init< const char* >())
		.def("AddEventListener", AddEventListener)
		.def("AddEventListener", AddEventListenerDefault)
		.def("AppendChild", &ElementInterface::AppendChild)
		.def("Blur", &Element::Blur)
		.def("Click", &Element::Click)
		.def("DispatchEvent", &ElementInterface::DispatchEvent)
		.def("Focus", &Element::Focus)
		.def("GetAttribute", python::make_function(&ElementInterface::GetAttribute, python::return_value_policy< python::return_by_value >()))
		.def("GetElementById", &Element::GetElementById, python::return_value_policy< python::return_by_value >())
		.def("GetElementsByTagName", &ElementInterface::GetElementsByTagName)
		.def("HasAttribute", &Element::HasAttribute)
		.def("HasChildNodes", &Element::HasChildNodes)
		.def("IsPseudoClassSet", &Element::IsPseudoClassSet)
		.def("InsertBefore", &Element::InsertBefore)
		.def("RemoveAttribute", &Element::RemoveAttribute)
		.def("RemoveChild", &Element::RemoveChild)		
		.def("ReplaceChild", &Element::ReplaceChild)
		.def("ScrollIntoView", &Element::ScrollIntoView)
		.def("SetAttribute", &ElementInterface::SetAttribute)
		.def("SetPseudoClass", &Element::SetPseudoClass)
		.def("SetClass", &Element::SetClass)
		.def("IsClassSet", &Element::IsClassSet)
		.add_property("absolute_left", &Element::GetAbsoluteLeft)
		.add_property("absolute_top", &Element::GetAbsoluteTop)
		.add_property("address", python::make_function(&ElementInterface::GetAddress, python::return_value_policy< python::return_by_value >()))
		.add_property("attributes", &ElementInterface::GetAttributes)
		.add_property("child_nodes", &ElementInterface::GetChildren)
		.add_property("class_name", python::make_function(&Element::GetClassNames, python::return_value_policy< python::return_by_value >()), &Element::SetClassNames)
		.add_property("client_left", &Element::GetClientLeft)
		.add_property("client_height", &Element::GetClientHeight)
		.add_property("client_top", &Element::GetClientTop)
		.add_property("client_width", &Element::GetClientWidth)
		.add_property("first_child", python::make_function(&Element::GetFirstChild, python::return_value_policy< python::return_by_value >()))
		.add_property("id", python::make_function(&Element::GetId, python::return_value_policy< python::return_by_value >()), &Element::SetId)
		.add_property("inner_rml", &ElementInterface::GetInnerRML, &Element::SetInnerRML)
		.add_property("last_child", python::make_function(&Element::GetLastChild, python::return_value_policy< python::return_by_value >()))
		.add_property("next_sibling", python::make_function(&Element::GetNextSibling, python::return_value_policy< python::return_by_value >()))
		.add_property("offset_height", &Element::GetOffsetHeight)
		.add_property("offset_left", &Element::GetOffsetLeft)
		.add_property("offset_parent", python::make_function(&Element::GetOffsetParent, python::return_value_policy< python::return_by_value >()))
		.add_property("offset_top", &Element::GetOffsetTop)
		.add_property("offset_width", &Element::GetOffsetWidth)
		.add_property("owner_document", python::make_function(&Element::GetOwnerDocument, python::return_value_policy< python::return_by_value >()))
		.add_property("parent_node", python::make_function(&Element::GetParentNode, python::return_value_policy< python::return_by_value >()))
		.add_property("previous_sibling", python::make_function(&Element::GetPreviousSibling, python::return_value_policy< python::return_by_value >()))
		.add_property("scroll_height", &Element::GetScrollHeight)
		.add_property("scroll_left", &Element::GetScrollLeft, &Element::SetScrollLeft)
		.add_property("scroll_top", &Element::GetScrollTop, &Element::SetScrollTop)
		.add_property("scroll_width", &Element::GetScrollWidth)
		.add_property("style", &ElementInterface::GetStyle)
		.add_property("tag_name", python::make_function(&Element::GetTagName, python::return_value_policy< python::return_by_value >()))
		.ptr();

	// Define the document type.
	class_definitions["Document"] = python::class_< ElementDocument, ElementDocumentWrapper, python::bases< Element >, boost::noncopyable >("Document", python::init< const char* >())
		.def("PullToFront", &ElementDocument::PullToFront)
		.def("PushToBack", &ElementDocument::PushToBack)
		.def("Show", &ElementInterface::Show)
		.def("Show", &ElementDocument::Show)
		.def("Hide", &ElementDocument::Hide)
		.def("Close", &ElementDocument::Close)
		.def("CreateElement", &ElementInterface::CreateElement)
		.def("CreateTextNode", &ElementInterface::CreateTextNode)
		.add_property("title", python::make_function(&ElementDocument::GetTitle, python::return_value_policy< python::return_by_value >()), &ElementDocument::SetTitle)
		.add_property("context", python::make_function(&ElementDocument::GetContext, python::return_value_policy< python::return_by_value >()))
		.ptr();

	// The ElementText type.
	python::class_< ElementText, ElementWrapper< ElementText >, boost::noncopyable, python::bases< Element > >("IElementText", python::no_init);
	class_definitions["Text"] = python::class_< ElementTextDefault, ElementWrapper< ElementTextDefault >, boost::noncopyable, python::bases< ElementText > >("Text", python::init< const char* >())
		.add_property("text", &ElementInterface::GetText, &ElementInterface::SetText)
		.ptr();

	// The ElementImage type.
	class_definitions["Image"] = python::class_< ElementImage, ElementWrapper< ElementImage >, boost::noncopyable, python::bases< Element > >("Image", python::init< const char* >())
		.ptr();

	// The ElementHandle type.
	class_definitions["Handle"] = python::class_< ElementHandle, ElementWrapper< ElementHandle >, boost::noncopyable, python::bases< Element > >("Handle", python::init< const char* >())
		.ptr();
}

void ElementInterface::InitialiseRocketInterface()
{
	// Redefine the generic instancer
	Factory::RegisterElementInstancer("*", new ElementInstancer((*class_definitions.find("Element")).second))->RemoveReference();
	Factory::RegisterElementInstancer("#text", new ElementInstancer((*class_definitions.find("Text")).second))->RemoveReference();
	Factory::RegisterElementInstancer("body", new ElementInstancer((*class_definitions.find("Document")).second))->RemoveReference();
	Factory::RegisterElementInstancer("handle", new ElementInstancer((*class_definitions.find("Handle")).second))->RemoveReference();
	Factory::RegisterElementInstancer("img", new ElementInstancer((*class_definitions.find("Image")).second))->RemoveReference();
}

// Get the element's address.
Rocket::Core::String ElementInterface::GetAddress(Element* element)
{
	return element->GetAddress(false);
}

ElementStyleProxy ElementInterface::GetStyle(Element* element)
{
	return ElementStyleProxy(element);
}

ElementChildrenProxy ElementInterface::GetChildren(Element* element)
{
	return ElementChildrenProxy(element);
}

ElementAttributeProxy ElementInterface::GetAttributes(Element* element)
{
	return ElementAttributeProxy(element);
}

void ElementInterface::AddEventListener(Element* element, const char* event, EventListener* listener, bool in_capture_phase)
{
	element->AddEventListener(event, listener, in_capture_phase);
}

void ElementInterface::AddEventListener(Element* element, const char* event, EventListener* listener)
{
	element->AddEventListener(event, listener);
}

// Override for AppendChild without the non-DOM boolean.
void ElementInterface::AppendChild(Element* element, Element* child)
{
	element->AppendChild(child);
}

void ElementInterface::DispatchEvent(Element* element, const char* event, const python::dict& parameters, bool interruptible)
{
	Rocket::Core::Dictionary ROCKET_parameters;

	PyObject* keys = PyDict_Keys(parameters.ptr());
	int num_keys = PyList_Size(keys);
	for (int i = 0; i < num_keys; ++i)
	{
		PyObject* py_key = PyList_GetItem(keys, i);
		if (!PyString_Check(py_key))
		{
			Py_DECREF(keys);
			PyErr_SetString(PyExc_KeyError, "Only string keys supported.");
			python::throw_error_already_set();
		}

		Rocket::Core::Variant value;
		if (!Rocket::Core::Python::Utilities::ConvertToVariant(value, PyDict_GetItem(parameters.ptr(), py_key)))
		{
			Py_DECREF(keys);
			PyErr_SetString(PyExc_ValueError, "Unable to convert parameter value.");
			python::throw_error_already_set();
		}

		const char* key = PyString_AsString(py_key);
		ROCKET_parameters.Set(key, value);
	}

	element->DispatchEvent(event, ROCKET_parameters, interruptible);
}

Rocket::Core::String ElementInterface::GetAttribute(Element* element, const char* name)
{
	return element->GetAttribute< Rocket::Core::String >(name, "");
}

// Returns the list of elements.
ElementList ElementInterface::GetElementsByTagName(Element* element, const char* tag)
{
	ElementList elements;
	element->GetElementsByTagName(elements, tag);
	return elements;
}

void ElementInterface::SetAttribute(Element* element, const char* name, const char* value)
{
	element->SetAttribute(name, value);
}

Rocket::Core::String ElementInterface::GetInnerRML(Element* element)
{
	Rocket::Core::String rml;
	element->GetInnerRML(rml);
	return rml;
}

Rocket::Core::String ElementInterface::GetText(ElementText* element)
{
	Rocket::Core::String text;
	element->GetText().ToUTF8(text);
	return text;
}

void ElementInterface::SetText(ElementText* element, const char* text)
{
	element->SetText(text);
}

void ElementInterface::Show(ElementDocument* document)
{
	document->Show();
}

// Creates a correctly reference-counted element.
python::object ElementInterface::CreateElement(ElementDocument* document, const char* tag)
{
	Element* new_element = document->CreateElement(tag);
	if (new_element == NULL)
		return python::object();

	python::object py_element = Rocket::Core::Python::Utilities::MakeObject(new_element);
	new_element->RemoveReference();
	return py_element;
}

// Creates a correctly reference-counted text node.
python::object ElementInterface::CreateTextNode(ElementDocument* document, const char* text)
{
	Element* new_element = document->CreateTextNode(text);
	if (new_element == NULL)
		return python::object();

	python::object py_element = Rocket::Core::Python::Utilities::MakeObject(new_element);
	new_element->RemoveReference();
	return py_element;
}

}
}
}
