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

#ifndef ROCKETCOREEVENTINSTANCER_H
#define ROCKETCOREEVENTINSTANCER_H

#include <Rocket/Core/ReferenceCountable.h>
#include <Rocket/Core/Header.h>

namespace Rocket {
namespace Core {

class Element;
class Event;

/**
	Abstract instancer interface for instancing events. This is required to be overridden for scripting systems.

	@author Lloyd Weehuizen
 */

class ROCKETCORE_API EventInstancer : public ReferenceCountable
{
public:
	virtual ~EventInstancer();

	/// Instance an event object.
	/// @param[in] target Target element of this event.
	/// @param[in] name Name of this event.
	/// @param[in] parameters Additional parameters for this event.
	/// @param[in] interruptible If the event propagation can be stopped.
	virtual Event* InstanceEvent(Element* target, const String& name, const Dictionary& parameters, bool interruptible) = 0;

	/// Releases an event instanced by this instancer.
	/// @param[in] event The event to release.
	virtual void ReleaseEvent(Event* event) = 0;

	/// Releases this event instancer.
	virtual void Release() = 0;

private:
	virtual void OnReferenceDeactivate();
};

}
}

#endif
