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

#ifndef ROCKETCOREPYTHONEVENTLISTENER_H
#define ROCKETCOREPYTHONEVENTLISTENER_H

#include <Rocket/Core/EventListener.h>

namespace Rocket {
namespace Core {
namespace Python {

/**
	Python Version of the Event Listener

	This event listener reference counts its OnAttach/OnDetach events
	and cleans itself up when the count hits 0.

	@author Lloyd Weehuizen
 */

class EventListener : public Rocket::Core::EventListener
{
public:
	/// Constructor from python object
	/// @param Python object to fire the event on
	EventListener( PyObject* object );
	/// Constructor for inline string
	/// @param code Python code string to fire
	/// @param context Optional, if set the code will be executed in this element's namespace
	EventListener( const Rocket::Core::String& code, Element* context = NULL );
	virtual ~EventListener();

	/// Process the event
	virtual void ProcessEvent( Event& event );

	/// Listen for detach event so we can clean up
	virtual void OnAttach( Element* element );
	virtual void OnDetach( Element* element );

private:
	// Element we're attached to
	Element* element;

	// Callable this listener is using
	PyObject* callable;
	PyObject* global_namespace;

	// Source code, if any
	Rocket::Core::String source_code;

	// Compile the source code
	bool Compile();
	PyObject* GetGlobalNamespace();
};

}
}
}

#endif
