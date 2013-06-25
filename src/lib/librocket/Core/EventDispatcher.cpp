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
#include "EventDispatcher.h"
#include <Rocket/Core/Element.h>
#include <Rocket/Core/Event.h>
#include <Rocket/Core/EventListener.h>
#include <Rocket/Core/Factory.h>

namespace Rocket {
namespace Core {

EventDispatcher::EventDispatcher(Element* _element)
{
	element = _element;
}

EventDispatcher::~EventDispatcher()
{
	// Detach from all event dispatchers
	for (Events::iterator event_itr = events.begin(); event_itr != events.end(); ++event_itr)
	{
		for (Listeners::iterator listener_itr = (*event_itr).second.begin(); listener_itr != (*event_itr).second.end(); ++listener_itr)
		{
			(*listener_itr).listener->OnDetach(element);
		}
	}
}

void EventDispatcher::AttachEvent(const String& type, EventListener* listener, bool in_capture_phase)
{
	// Look up the event
	Events::iterator event_itr = events.find(type);

	// Ensure the event is in the event list
	if (event_itr == events.end())
	{
		event_itr = events.insert(std::pair< String, Listeners >(type, Listeners())).first;
	}

	// Add the action to the events
	(*event_itr).second.push_back(Listener(listener, in_capture_phase));

	listener->OnAttach(element);
}

void EventDispatcher::DetachEvent(const String& type, EventListener* listener, bool in_capture_phase)
{
	// Look up the event
	Events::iterator event_itr = events.find(type);

	// Bail if we can't find the event
	if (event_itr == events.end())
	{
		return;
	}

	// Find the relevant listener and erase it
	Listeners::iterator listener_itr = (*event_itr).second.begin();
	while (listener_itr != (*event_itr).second.end())
	{
		if ((*listener_itr).listener == listener && (*listener_itr).in_capture_phase == in_capture_phase)
		{
			listener_itr = (*event_itr).second.erase(listener_itr);
			listener->OnDetach(element);
		}
		else
			++listener_itr;
	}
}

// Detaches all events from this dispatcher and all child dispatchers.
void EventDispatcher::DetachAllEvents()
{
	for (Events::iterator event_iterator = events.begin(); event_iterator != events.end(); ++event_iterator)
	{
		Listeners& listeners = event_iterator->second;
		for (size_t i = 0; i < listeners.size(); ++i)
			listeners[i].listener->OnDetach(element);
	}

	events.clear();

	for (int i = 0; i < element->GetNumChildren(true); ++i)
		element->GetChild(i)->GetEventDispatcher()->DetachAllEvents();
}

bool EventDispatcher::DispatchEvent(Element* target_element, const String& name, const Dictionary& parameters, bool interruptible)
{
	//Event event(target_element, name, parameters, interruptible);
	Event* event = Factory::InstanceEvent(target_element, name, parameters, interruptible);
	if (event == NULL)
		return false;

	// Build the element traversal from the tree
	typedef std::vector<Element*> Elements;
	Elements elements;

	Element* walk_element = target_element->GetParentNode();
	while (walk_element) 
	{
		elements.push_back(walk_element);
		walk_element = walk_element->GetParentNode();
	}

	event->SetPhase(Event::PHASE_CAPTURE);
	// Capture phase - root, to target (only events that have registered as capture events)
	// Note: We walk elements in REVERSE as they're placed in the list from the elements parent to the root
	for (int i = elements.size() - 1; i >= 0 && event->IsPropagating(); i--) 
	{
		EventDispatcher* dispatcher = elements[i]->GetEventDispatcher();
		event->SetCurrentElement(elements[i]);
		dispatcher->TriggerEvents(event);
	}

	// Target phase - direct at the target
	if (event->IsPropagating()) 
	{
		event->SetPhase(Event::PHASE_TARGET);
		event->SetCurrentElement(target_element);
		TriggerEvents(event);
	}

	if (event->IsPropagating()) 
	{
		event->SetPhase(Event::PHASE_BUBBLE);
		// Bubble phase - target to root (normal event bindings)
		for (size_t i = 0; i < elements.size() && event->IsPropagating(); i++) 
		{
			EventDispatcher* dispatcher = elements[i]->GetEventDispatcher();
			event->SetCurrentElement(elements[i]);
			dispatcher->TriggerEvents(event);
		}
	}

	bool propagating = event->IsPropagating();
	event->RemoveReference();
	return propagating;
}

void EventDispatcher::TriggerEvents(Event* event)
{
	// Look up the event
	Events::iterator itr = events.find(event->GetType());

	if (itr != events.end())
	{
		// Dispatch all actions
		Listeners& listeners = (*itr).second;
		if (event->GetPhase() == Event::PHASE_TARGET)
		{
			// Fire all listeners waiting for bubble events before we send the event to the target itself.
			for (size_t i = 0; i < listeners.size() && event->IsPropagating(); i++) 
			{
				if (!listeners[i].in_capture_phase)
				{
					listeners[i].listener->ProcessEvent(*event);
				}
			}

			// Send the event to the target element itself.
			if (event->IsPropagating())
				element->ProcessEvent(*event);

			// Fire all listeners waiting for capture events.
			for (size_t i = 0; i < listeners.size() && event->IsPropagating(); i++) 
			{
				if (listeners[i].in_capture_phase)
					listeners[i].listener->ProcessEvent(*event);
			}

			return;
		}
		else
		{
			bool in_capture_phase = event->GetPhase() == Event::PHASE_CAPTURE;

			for (size_t i = 0; i < listeners.size() && event->IsPropagating(); i++) 
			{
				// If we're in the correct phase, fire the event
				if (listeners[i].in_capture_phase == in_capture_phase)
					listeners[i].listener->ProcessEvent(*event);
			}
		}
	}

	if (event->GetPhase() != Event::PHASE_CAPTURE)
	{
		// Send the event to the target element.
		element->ProcessEvent(*event);
	}
}

}
}
