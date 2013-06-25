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

#ifndef ROCKETDEBUGGERELEMENTINFO_H
#define ROCKETDEBUGGERELEMENTINFO_H

#include <Rocket/Core/ElementDocument.h>
#include <Rocket/Core/EventListener.h>

namespace Rocket {
namespace Debugger {

typedef std::pair< Core::String, const Core::Property* > NamedProperty;
typedef std::vector< NamedProperty > NamedPropertyList;
typedef std::map< Core::PseudoClassList, NamedPropertyList > NamedPropertyMap;

/**
	@author Robert Curry
 */

class ElementInfo : public Core::ElementDocument, public Core::EventListener
{
public:
	ElementInfo(const Core::String& tag);
	virtual ~ElementInfo();

	/// Initialises the info element.
	/// @return True if the element initialised successfully, false otherwise.
	bool Initialise();
	/// Clears the element references.
	void Reset();

	/// Called when an element is destroyed.
	void OnElementDestroy(Core::Element* element);

	void RenderHoverElement();
	void RenderSourceElement();

protected:
	virtual void ProcessEvent(Core::Event& event);

private:
	void SetSourceElement(Core::Element* new_source_element);
	void UpdateSourceElement();

	void BuildElementPropertiesRML(Core::String& property_rml, Core::Element* element, Core::Element* primary_element);
	void BuildPropertiesRML(Core::String& property_rml, const NamedPropertyList& properties);
	void BuildPropertyRML(Core::String& property_rml, const Core::String& name, const Core::Property* property);

	void RemoveTrailingZeroes(Core::String& string);

	bool IsDebuggerElement(Core::Element* element);

	Core::Element* hover_element;
	Core::Element* source_element;
};

}
}

#endif
