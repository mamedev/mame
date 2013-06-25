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

#include <Rocket/Controls/ElementTabSet.h>
#include <Rocket/Core/Math.h>
#include <Rocket/Core/Factory.h>

namespace Rocket {
namespace Controls {

ElementTabSet::ElementTabSet(const Rocket::Core::String& tag) : Core::Element(tag)
{
	active_tab = 0;
}

ElementTabSet::~ElementTabSet()
{
}

// Sets the specifed tab index's tab title RML.
void ElementTabSet::SetTab(int tab_index, const Rocket::Core::String& rml)
{
	Core::Element* element = Core::Factory::InstanceElement(NULL, "*", "tab", Rocket::Core::XMLAttributes());
	Core::Factory::InstanceElementText(element, rml);
	SetTab(tab_index, element);
}

// Sets the specifed tab index's tab panel RML.
void ElementTabSet::SetPanel(int tab_index, const Rocket::Core::String& rml)
{
	Core::Element* element = Core::Factory::InstanceElement(NULL, "*", "tab", Rocket::Core::XMLAttributes());
	Core::Factory::InstanceElementText(element, rml);
	SetPanel(tab_index, element);
}

// Set the specifed tab index's title element.
void ElementTabSet::SetTab(int tab_index, Core::Element* element)
{	
	Core::Element* tabs = GetChildByTag("tabs");
	if (tab_index >= 0 &&
		tab_index < tabs->GetNumChildren())
		tabs->ReplaceChild(GetChild(tab_index), element);
	else
		tabs->AppendChild(element);
}

// Set the specified tab index's body element.
void ElementTabSet::SetPanel(int tab_index, Core::Element* element)
{	
	// Append the window
	Core::Element* windows = GetChildByTag("panels");
	if (tab_index >= 0 &&
		tab_index < windows->GetNumChildren())
		windows->ReplaceChild(GetChild(tab_index), element);
	else
		windows->AppendChild(element);
}

// Remove one of the tab set's panels and its corresponding tab.
void ElementTabSet::RemoveTab(int tab_index)
{
	if (tab_index < 0)
		return;

	Core::Element* panels = GetChildByTag("panels");
	Core::Element* tabs = GetChildByTag("tabs");

	if (panels->GetNumChildren() > tab_index &&
		tabs->GetNumChildren() > tab_index)
	{
		panels->RemoveChild(panels->GetChild(tab_index));
		tabs->RemoveChild(tabs->GetChild(tab_index));
	}
}

// Retrieve the number of tabs in the tabset.
int ElementTabSet::GetNumTabs()
{
	return GetChildByTag("tabs")->GetNumChildren();
}

void ElementTabSet::SetActiveTab(int tab_index)
{
	// Update display if the tab has changed
	if (tab_index != active_tab)
	{
		Core::Element* tabs = GetChildByTag("tabs");
		Core::Element* old_tab = tabs->GetChild(active_tab);
		Core::Element* new_tab = tabs->GetChild(tab_index);

		if (old_tab)
			old_tab->SetPseudoClass("selected", false);
		if (new_tab)
			new_tab->SetPseudoClass("selected", true);

		Core::Element* windows = GetChildByTag("panels");
		Core::Element* old_window = windows->GetChild(active_tab);
		Core::Element* new_window = windows->GetChild(tab_index);

		if (old_window)
			old_window->SetProperty("display", "none");			
		if (new_window)
			new_window->SetProperty("display", "inline-block");

		active_tab = tab_index;

		Rocket::Core::Dictionary parameters;
		parameters.Set("tab_index", active_tab);
		DispatchEvent("tabchange", parameters);
	}
}

int ElementTabSet::GetActiveTab() const
{
	return active_tab;
}

// Process the incoming event.
void ElementTabSet::ProcessEvent(Core::Event& event)
{
	Core::Element::ProcessEvent(event);

	if (event.GetCurrentElement() == this && event == "click")
	{
		// Find the tab that this click occured on
		Core::Element* tabs = GetChildByTag("tabs");
		Core::Element* tab = event.GetTargetElement();
		while (tab && tab != this && tab->GetParentNode() != tabs)
			tab = tab->GetParentNode();

		// Abort if we couldn't find the tab the click occured on
		if (!tab || tab == this)
			return;

		// Determine the new active tab index
		int new_active_tab = active_tab;
		for (int i = 0; i < tabs->GetNumChildren(); i++)
		{
			if (tabs->GetChild(i) == tab)
			{
				new_active_tab = i;
				break;
			}
		}

		SetActiveTab(new_active_tab);
	}
}

void ElementTabSet::OnChildAdd(Core::Element* child)
{
	Core::Element::OnChildAdd(child);

	if (child->GetParentNode() == GetChildByTag("tabs"))
	{
		// Set up the new button and append it
		child->SetProperty("display", "inline-block");
		child->AddEventListener("click", this);

		if (child->GetParentNode()->GetChild(active_tab) == child)
			child->SetPseudoClass("selected", true);
	}

	if (child->GetParentNode() == GetChildByTag("panels"))
	{
		// Hide the new tab window
		child->SetProperty("display", "none");
		
		// Make the new element visible if its the active tab
		if (child->GetParentNode()->GetChild(active_tab) == child)
			child->SetProperty("display", "inline-block");
	}
}

void ElementTabSet::OnChildRemove(Core::Element* child)
{
	Core::Element::OnChildRemove(child);

	// If its a tab, remove its event listener
	if (child->GetParentNode() == GetChildByTag("tabs"))
	{
		child->RemoveEventListener("click", this);
	}
}

Core::Element* ElementTabSet::GetChildByTag(const Rocket::Core::String& tag)
{
	// Look for the existing child
	for (int i = 0; i < GetNumChildren(); i++)
	{
		if (GetChild(i)->GetTagName() == tag)
			return GetChild(i);
	}

	// If it doesn't exist, create it
	Core::Element* element = Core::Factory::InstanceElement(this, "*", tag, Rocket::Core::XMLAttributes());
	AppendChild(element);
	element->RemoveReference();
	return element;
}

}
}
