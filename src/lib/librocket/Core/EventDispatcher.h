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

#ifndef ROCKETCOREEVENTDISPATCHER_H
#define ROCKETCOREEVENTDISPATCHER_H

#include <Rocket/Core/String.h>
#include <Rocket/Core/Event.h>
#include <map>

namespace Rocket {
namespace Core {

class Element;
class EventListener;

/**
	The Event Dispatcher manages a list of event listeners (based on URL) and triggers the events via EventHandlers
	whenever requested.

	@author Lloyd Weehuizen
*/

class EventDispatcher 
{
public:
	/// Constructor
	/// @param element Element this dispatcher acts on
	EventDispatcher(Element* element);

	// Destructor
	~EventDispatcher();

	/// Attaches a new listener to the specified event name
	/// @param[in] type Type of the event to attach to
	/// @param[in] event_listener The event listener to be notified when the event fires
	/// @param[in] in_capture_phase Should the listener be notified in the capture phase
	void AttachEvent(const String& type, EventListener* event_listener, bool in_capture_phase);

	/// Detaches a listener from the specified event name
	/// @param[in] type Type of the event to attach to
	/// @para[in]m event_listener The event listener to be notified when the event fires
	/// @param[in] in_capture_phase Should the listener be notified in the capture phase
	void DetachEvent(const String& type, EventListener* listener, bool in_capture_phase);

	/// Detaches all events from this dispatcher and all child dispatchers.
	void DetachAllEvents();

	/// Dispatches the specified event with element as the target
	/// @param[in] target_element The target element of the event
	/// @param[in] name The name of the event
	/// @param[in] parameters The event parameters
	/// @param[in] interruptible Can the event propagation be stopped
	/// @return True if the event was not consumed (ie, was prevented from propagating by an element), false if it was.
	bool DispatchEvent(Element* element, const String& name, const Dictionary& parameters, bool interruptible);

private:
	Element* element;

	struct Listener
	{
		Listener(EventListener* _listener, bool _in_capture_phase) : listener(_listener), in_capture_phase(_in_capture_phase) {}
		EventListener* listener;
		bool in_capture_phase;
	};
	typedef std::vector< Listener > Listeners;
	typedef std::map< String, Listeners > Events;
	Events events;

	void TriggerEvents(Event* event);
};

}
}

#endif
