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

#ifndef ROCKETCOREPYTHONELEMENTINTERFACE_H
#define ROCKETCOREPYTHONELEMENTINTERFACE_H

#include <Rocket/Core/Element.h>

#include "ElementStyleProxy.h"
#include "ElementChildrenProxy.h"
#include "ElementAttributeProxy.h"
#include "EventListener.h"

namespace Rocket {
namespace Core {
namespace Python {

/**
	Python interface to the element class

	Contains a list of static methods that wrap
	element functions for easier use from python.

	@author Lloyd Weehuizen
 */

class ElementInterface
{
public:
	/// Initialises the python element interface
	static void InitialisePythonInterface();
	/// Initialise the rocket element interface
	static void InitialiseRocketInterface();

	/// Get the element's address.
	static Rocket::Core::String GetAddress(Element* element);
	/// Get a style proxy object for the element
	static ElementStyleProxy GetStyle(Element* element);
	/// Get a children proxy object for the element
	static ElementChildrenProxy GetChildren(Element* element);
	/// Get an attribute proxy for this element
	static ElementAttributeProxy GetAttributes(Element* element);
	/// Override for AddEventListener that takes a python EventListener
	static void AddEventListener(Element* element, const char* event, Rocket::Core::Python::EventListener* listener, bool in_capture_phase);
	/// Override for AddEventListener without the third parameter.
	static void AddEventListener(Element* element, const char* event, Rocket::Core::Python::EventListener* listener);
	/// Override for AppendChild without the non-DOM boolean.
	static void AppendChild(Element* element, Element* child);
	/// Dispatches an event.
	static void DispatchEvent(Element* element, const char* event, const python::dict& parameters, bool interruptible);
	/// Get the specified attribute
	static Rocket::Core::String GetAttribute(Element* element, const char* name);
	/// Returns the list of elements.
	static ElementList GetElementsByTagName(Element* element, const char* tag);
	/// Override for RemoveEventListener without the third parameter.
	static void RemoveEventListener(Element* element, const char* event, EventListener* listener);
	/// Get the specified attribute
	static void SetAttribute(Element* element, const char* name, const char* value);
	/// Get the inner html of the given element
	static Rocket::Core::String GetInnerRML(Element* element);

	/// ElementText interface.
	static Rocket::Core::String GetText(ElementText* element);
	static void SetText(ElementText* element, const char* text);

	/// Document Interface.
	
	/// Show a document - override for default parameters
	static void Show(ElementDocument* document);
	/// Creates a correctly reference-counted element.
	static python::object CreateElement(ElementDocument* document, const char* tag);
	/// Creates a correctly reference-counted text node.
	static python::object CreateTextNode(ElementDocument* document, const char* text);
};

}
}
}

#endif
