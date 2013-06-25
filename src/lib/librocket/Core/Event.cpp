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
#include <Rocket/Core/Event.h>
#include <Rocket/Core/EventInstancer.h>

namespace Rocket {
namespace Core {

Event::Event()
{
	phase = PHASE_UNKNOWN;
	interruped = false;
	interruptible = false;
	current_element = NULL;
	target_element = NULL;
}

Event::Event(Element* _target_element, const String& _type, const Dictionary& _parameters, bool _interruptible) : type(_type), parameters(_parameters), target_element(_target_element), interruptible(_interruptible)
{
	phase = PHASE_UNKNOWN;
	interruped = false;
	current_element = NULL;
}

Event::~Event()
{
}

void Event::SetCurrentElement(Element* element)
{
	current_element = element;
}

Element* Event::GetCurrentElement() const
{
	return current_element;
}

Element* Event::GetTargetElement() const
{
	return target_element;
}

const String& Event::GetType() const
{
	return type;
}

bool Event::operator==(const String& _type) const
{
	return type == _type;
}

void Event::SetPhase(EventPhase _phase)
{
	phase = _phase;
}

Event::EventPhase Event::GetPhase() const
{
	return phase;
}

bool Event::IsPropagating() const
{
	return !interruped;
}

void Event::StopPropagation()
{
	// Set interruped to true if we can be interruped
	if (interruptible) 
	{
		interruped = true;
	}
}

const Dictionary* Event::GetParameters() const
{
	return &parameters;
}

void Event::OnReferenceDeactivate()
{
	instancer->ReleaseEvent(this);
}

}
}
