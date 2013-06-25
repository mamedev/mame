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

#ifndef ROCKETCONTROLSELEMENTTABSET_H
#define ROCKETCONTROLSELEMENTTABSET_H

#include <Rocket/Core/Element.h>
#include <Rocket/Core/EventListener.h>
#include <Rocket/Controls/Header.h>

namespace Rocket {
namespace Controls {

/**
	A tabulated set of panels.

	@author Lloyd Weehuizen
 */

class ROCKETCONTROLS_API ElementTabSet : public Core::Element, public Core::EventListener
{
public:
	ElementTabSet(const Rocket::Core::String& tag);
	~ElementTabSet();

	/// Sets the specifed tab index's tab title RML.
	/// @param[in] tab_index The tab index to set. If it doesn't already exist, it will be created.
	/// @param[in] rml The RML to set on the tab title.
	void SetTab(int tab_index, const Rocket::Core::String& rml);
	/// Sets the specifed tab index's tab panel RML.
	/// @param[in] tab_index The tab index to set. If it doesn't already exist, it will be created.
	/// @param[in] rml The RML to set on the tab panel.
	void SetPanel(int tab_index, const Rocket::Core::String& rml);

	/// Set the specifed tab index's title element.
	/// @param[in] tab_index The tab index to set. If it doesn't already exist, it will be created.
	/// @param[in] element The root of the element tree to set as the tab title.
	void SetTab(int tab_index, Core::Element* element);
	/// Set the specified tab index's body element.
	/// @param[in] tab_index The tab index to set. If it doesn't already exist, it will be created.
	/// @param[in] element The root of the element tree to set as the window.
	void SetPanel(int tab_index, Core::Element* element);

	/// Remove one of the tab set's panels and its corresponding tab.
	/// @param[in] tab_index The tab index to remove. If no tab matches this index, nothing will be removed.
	void RemoveTab(int tab_index);

	/// Retrieve the number of tabs in the tabset.
	/// @return The number of tabs.
	int GetNumTabs();

	/// Sets the currently active (visible) tab index.
	/// @param[in] tab_index Index of the tab to display.
	void SetActiveTab(int tab_index);

	/// Get the current active tab index.
	/// @return The index of the active tab.
	int GetActiveTab() const;

	/// Process the incoming event.
	void ProcessEvent(Core::Event& event);

protected:
	// Catch child add/removes so we can correctly set up their events.
	virtual void OnChildAdd(Core::Element* child);
	virtual void OnChildRemove(Core::Element* child);

private:
	Core::Element* GetChildByTag(const Rocket::Core::String& tag);

	int active_tab;
};

}
}

#endif
