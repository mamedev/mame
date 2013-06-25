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

#ifndef ROCKETCOREEVENT_H
#define ROCKETCOREEVENT_H

#include <Rocket/Core/Dictionary.h>
#include <Rocket/Core/ScriptInterface.h>
#include <Rocket/Core/Header.h>

namespace Rocket {
namespace Core {

class Element;
class EventInstancer;

/**
	An event that propogates through the element hierarchy. Events follow the DOM3 event specification. See
	http://www.w3.org/TR/DOM-Level-3-Events/events.html.

	@author Lloyd Weehuizen
 */

class ROCKETCORE_API Event : public ScriptInterface
{
public:
	/// Constructor
	Event();
	/// Constructor
	/// @param[in] target The target element of this event
	/// @param[in] type The event type
	/// @param[in] parameters The event parameters
	/// @param[in] interruptible Can this event have is propagation stopped?
	Event(Element* target, const String& type, const Dictionary& parameters, bool interruptible = false);
	/// Destructor
	virtual ~Event();

	enum EventPhase { PHASE_UNKNOWN, PHASE_CAPTURE, PHASE_TARGET, PHASE_BUBBLE };

	/// Get the current propagation phase.
	/// @return Current phase the event is in.
	EventPhase GetPhase() const;
	/// Set the current propagation phase
	/// @param phase Switch the phase the event is in
	void SetPhase(EventPhase phase);

	/// Set the current element in the propagation.
	/// @param[in] element The current element.
	void SetCurrentElement(Element* element);
	/// Get the current element in the propagation.
	/// @return The current element in propagation.
	Element* GetCurrentElement() const;

	/// Get the target element
	/// @return The target element of this event
	Element* GetTargetElement() const;

	/// Get the event type.
	/// @return The event type.
	const String& GetType() const;
	/// Checks if the event is of a certain type.
	/// @param type The name of the type to check for.
	/// @return True if the event is of the requested type, false otherwise.
	bool operator==(const String& type) const;

	/// Has the event been stopped?
	/// @return True if the event is still propogating
	bool IsPropagating() const;
	/// Stops the propagation of the event wherever it is
	void StopPropagation();

	/// Returns the value of one of the event's parameters.
	/// @param key[in] The name of the desired parameter.
	/// @return The value of the requested parameter.
	template < typename T >
	T GetParameter(const String& key, const T& default_value)
	{
		return parameters.Get(key, default_value);
	}
	/// Access the dictionary of parameters
	/// @return The dictionary of parameters
	const Dictionary* GetParameters() const;

	/// Release this event.
	virtual void OnReferenceDeactivate();

protected:
	String type;
	Dictionary parameters;

	Element* target_element;
	Element* current_element;

private:
	bool interruptible;
	bool interruped;

	EventPhase phase;

	EventInstancer* instancer;

	friend class Factory;
};

}
}

#endif
