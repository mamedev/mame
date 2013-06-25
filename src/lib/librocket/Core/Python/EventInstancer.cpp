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
#include <Rocket/Core/Python/Utilities.h>
#include "EventWrapper.h"
#include "EventInstancer.h"

namespace Rocket {
namespace Core {
namespace Python {

EventInstancer::EventInstancer(PyObject* _instancer)
{
	instancer = _instancer;
	Py_INCREF( _instancer );
}

EventInstancer::~EventInstancer()
{
	Py_DECREF( instancer );
}

Event* EventInstancer::InstanceEvent(Element* target, const Rocket::Core::String& name, const Rocket::Core::Dictionary& parameters, bool interruptable)
{
	EventWrapper* event = NULL;

	try
	{	
		// Convert parameters to a python representation
		python::object params(parameters);

		// Increase reference count on required args
		Py_INCREF(params.ptr());

		// Put the arguments into a tuple
		PyObject* args = PyTuple_New(3);
		PyTuple_SetItem(args, 0, PyString_FromString(name.CString()));
		PyTuple_SetItem(args, 1, params.ptr());
		PyTuple_SetItem(args, 2, PyBool_FromLong(interruptable));
	
		// Instance
		PyObject* instance = PyObject_CallObject(instancer, args);
		Py_DECREF(args);

		// Rebind the target element
		if (instance)
		{
			event = python::extract<EventWrapper*>(Rocket::Core::Python::Utilities::MakeObject(instance));
			if (event)
			{
				event->SetTargetElement(target);
			}
		}
	}
	catch (python::error_already_set&)
	{		
		event = NULL;
	}

	if (!event)
	{
		Rocket::Core::Python::Utilities::PrintError(true);
	}

	return event;
}

// Releases an event instanced by this instancer.
void EventInstancer::ReleaseEvent(Event* event)
{
	EventWrapper* event_wrapper = dynamic_cast< EventWrapper* >(event);
	if (event_wrapper != NULL)
	{
		// Little bit of footwork so that the destructors security check passes
		PyObject* _self = event_wrapper->self;
		event_wrapper->self = NULL;
		Py_DECREF( _self );
	}
}

void EventInstancer::Release()
{
	delete this;
}

}
}
}
