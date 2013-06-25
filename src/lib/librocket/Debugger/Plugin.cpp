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

#include "Plugin.h"
#include <Rocket/Core/Types.h>
#include <Rocket/Core.h>
#include "ElementContextHook.h"
#include "ElementInfo.h"
#include "ElementLog.h"
#include "FontSource.h"
#include "Geometry.h"
#include "MenuSource.h"
#include "SystemInterface.h"
#include <stack>

namespace Rocket {
namespace Debugger {

Plugin* Plugin::instance = NULL;

Plugin::Plugin()
{
	ROCKET_ASSERT(instance == NULL);
	instance = this;
	host_context = NULL;
	debug_context = NULL;
	log_hook = NULL;

	menu_element = NULL;
	info_element = NULL;
	log_element = NULL;

	render_outlines = false;
}

Plugin::~Plugin()
{
	instance = NULL;
}

// Initialises the debugging tools into the given context.
bool Plugin::Initialise(Core::Context* context)
{
	host_context = context;
	Geometry::SetContext(context);

	if (!LoadFont())
	{
		Core::Log::Message(Core::Log::LT_ERROR, "Failed to initialise debugger, unable to load font.");
		return false;
	}

	if (!LoadMenuElement() ||
		!LoadInfoElement() ||
		!LoadLogElement())
	{
		Core::Log::Message(Core::Log::LT_ERROR, "Failed to initialise debugger, error while load debugger elements.");
		return false;
	}

	Core::Factory::RegisterElementInstancer("debug-hook", new Core::ElementInstancerGeneric< ElementContextHook >())->RemoveReference();

	return true;
}

// Sets the context to be debugged.
bool Plugin::SetContext(Core::Context* context)
{
	// Remove the debug hook from the old context.
	if (debug_context != NULL &&
		hook_element != NULL)
	{
		debug_context->UnloadDocument(hook_element);
		hook_element->RemoveReference();
		hook_element = NULL;
	}

	// Add the debug hook into the new context.
	if (context != NULL)
	{
		Core::ElementDocument* element = context->CreateDocument("debug-hook");
		if (element == NULL)
			return false;

		hook_element = dynamic_cast< ElementContextHook* >(element);
		if (hook_element == NULL)
		{
			element->RemoveReference();
			context->UnloadDocument(element);
			return false;
		}

		hook_element->Initialise(this);
	}

	// Attach the info element to the new context.
	if (info_element != NULL)
	{
		if (debug_context != NULL)
		{
			debug_context->RemoveEventListener("click", info_element, true);
			debug_context->RemoveEventListener("mouseover", info_element, true);
		}

		if (context != NULL)
		{
			context->AddEventListener("click", info_element, true);
			context->AddEventListener("mouseover", info_element, true);
		}

		info_element->Reset();
	}

	debug_context = context;
	return true;
}

// Sets the visibility of the debugger.
void Plugin::SetVisible(bool visibility)
{
	if (visibility)
		menu_element->SetProperty("visibility", "visible");
	else
		menu_element->SetProperty("visibility", "hidden");
}

// Returns the visibility of the debugger.
bool Plugin::IsVisible()
{
	return menu_element->IsVisible();
}

// Renders any debug elements in the debug context.
void Plugin::Render()
{
	// Render the outlines of the debug context's elements.
	if (render_outlines &&
		debug_context != NULL)
	{
		for (int i = 0; i < debug_context->GetNumDocuments(); ++i)
		{
			Core::ElementDocument* document = debug_context->GetDocument(i);
			if (document->GetId().Find("rkt-debug-") == 0)
				continue;

			std::stack< Core::Element* > element_stack;
			element_stack.push(document);

			while (!element_stack.empty())
			{
				Core::Element* element = element_stack.top();
				element_stack.pop();

				for (int j = 0; j < element->GetNumBoxes(); ++j)
				{
					const Core::Box& box = element->GetBox(j);
					Geometry::RenderOutline(element->GetAbsoluteOffset(Core::Box::BORDER) + box.GetPosition(Core::Box::BORDER), box.GetSize(Core::Box::BORDER), Core::Colourb(255, 0, 0, 128), 1);
				}

				for (int j = 0; j < element->GetNumChildren(); ++j)
					element_stack.push(element->GetChild(j));
			}
		}
	}

	// Render the info element's boxes.
	if (info_element != NULL &&
		info_element->IsVisible())
	{
		info_element->RenderHoverElement();
		info_element->RenderSourceElement();
	}
}

// Called when Rocket shuts down.
void Plugin::OnShutdown()
{
	// Release the elements before we leak track, this ensures the debugger hook has been cleared
	// and that we don't try send the messages to the debug log window
	ReleaseElements();

	if (!elements.empty())
	{
		Core::Log::Message(Core::Log::LT_WARNING, "%u leaked elements detected.", elements.size());

		int count = 0;
		for (ElementInstanceMap::iterator i = elements.begin(); i != elements.end(); ++i)
			Core::Log::Message(Core::Log::LT_WARNING, "\t(%d) %s -> %s", count++, (*i)->GetTagName().CString(), (*i)->GetAddress().CString());
	}

	delete this;
}

// Called whenever a Rocket context is destroyed.
void Plugin::OnContextDestroy(Core::Context* context)
{
	if (context == debug_context)
	{
		// The context we're debugging is being destroyed, so we need to remove our debug hook elements.
		SetContext(NULL);
	}

	if (context == host_context)
	{
		// Our host is being destroyed, so we need to shut down the debugger.

		ReleaseElements();

		Geometry::SetContext(NULL);
		context = NULL;
	}
}

// Called whenever an element is created.
void Plugin::OnElementCreate(Core::Element* element)
{
	// Store the stack addresses for this frame.
	elements.insert(element);
}

// Called whenever an element is destroyed.
void Plugin::OnElementDestroy(Core::Element* element)
{
	elements.erase(element);

	if (info_element != NULL)
		info_element->OnElementDestroy(element);
}

// Event handler for events from the debugger elements.
void Plugin::ProcessEvent(Core::Event& event)
{
	if (event == "click")
	{
		if (event.GetTargetElement()->GetId() == "event-log-button")
		{
			if (log_element->IsVisible())
				log_element->SetProperty("visibility", "hidden");
			else
				log_element->SetProperty("visibility", "visible");
		}
		else if (event.GetTargetElement()->GetId() == "debug-info-button")
		{
			if (info_element->IsVisible())
				info_element->SetProperty("visibility", "hidden");
			else
				info_element->SetProperty("visibility", "visible");
		}
		else if (event.GetTargetElement()->GetId() == "outlines-button")
		{
			render_outlines = !render_outlines;
		}
	}
}

Plugin* Plugin::GetInstance()
{
	return instance;
}

bool Plugin::LoadFont()
{
	return (Core::FontDatabase::LoadFontFace(lacuna_regular, sizeof(lacuna_regular) / sizeof(unsigned char), "Lacuna", Core::Font::STYLE_NORMAL, Core::Font::WEIGHT_NORMAL) &&
			Core::FontDatabase::LoadFontFace(lacuna_italic, sizeof(lacuna_italic) / sizeof(unsigned char), "Lacuna", Core::Font::STYLE_ITALIC, Core::Font::WEIGHT_NORMAL));
}

bool Plugin::LoadMenuElement()
{
	menu_element = host_context->CreateDocument();
	if (menu_element == NULL)
		return false;

	menu_element->SetId("rkt-debug-menu");
	menu_element->SetProperty("visibility", "hidden");
	menu_element->SetInnerRML(menu_rml);

	// Remove our reference on the document.
	menu_element->RemoveReference();

	Core::StyleSheet* style_sheet = Core::Factory::InstanceStyleSheetString(menu_rcss);
	if (style_sheet == NULL)
	{
		host_context->UnloadDocument(menu_element);
		menu_element = NULL;

		return false;
	}

	menu_element->SetStyleSheet(style_sheet);
	style_sheet->RemoveReference();
	menu_element->AddReference();

	// Set the version info in the menu.
	menu_element->GetElementById("version-number")->SetInnerRML("v" + Rocket::Core::GetVersion());

	// Attach to the buttons.
	Core::Element* event_log_button = menu_element->GetElementById("event-log-button");
	event_log_button->AddEventListener("click", this);

	Core::Element* element_info_button = menu_element->GetElementById("debug-info-button");
	element_info_button->AddEventListener("click", this);

	Core::Element* outlines_button = menu_element->GetElementById("outlines-button");
	outlines_button->AddEventListener("click", this);

	return true;
}

bool Plugin::LoadInfoElement()
{
	Core::Factory::RegisterElementInstancer("debug-info", new Core::ElementInstancerGeneric< ElementInfo >())->RemoveReference();
	info_element = dynamic_cast< ElementInfo* >(host_context->CreateDocument("debug-info"));
	if (info_element == NULL)
		return false;

	info_element->SetProperty("visibility", "hidden");

	if (!info_element->Initialise())
	{
		info_element->RemoveReference();
		host_context->UnloadDocument(info_element);
		info_element = NULL;

		return false;
	}

	return true;
}

bool Plugin::LoadLogElement()
{
	Core::Factory::RegisterElementInstancer("debug-log", new Core::ElementInstancerGeneric< ElementLog >())->RemoveReference();
	log_element = dynamic_cast< ElementLog* >(host_context->CreateDocument("debug-log"));
	if (log_element == NULL)
		return false;

	log_element->SetProperty("visibility", "hidden");

	if (!log_element->Initialise())
	{
		log_element->RemoveReference();
		host_context->UnloadDocument(log_element);
		log_element = NULL;

		return false;
	}

	// Make the system interface; this will trap the log messages for us.
	log_hook = new SystemInterface(log_element);
	
	return true;
}

void Plugin::ReleaseElements()
{
	if (menu_element)
	{
		menu_element->RemoveReference();
		menu_element = NULL;
	}

	if (info_element)
	{
		info_element->RemoveReference();
		info_element = NULL;
	}

	if (log_element)
	{
		log_element->RemoveReference();
		log_element = NULL;
		delete log_hook;
	}

	if (hook_element)
	{
		hook_element->RemoveReference();
		hook_element = NULL;
	}
}

}
}
