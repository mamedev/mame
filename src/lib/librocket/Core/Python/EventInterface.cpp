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
#include "EventInstancer.h"

#include <Rocket/Core/Python/ConverterScriptObject.h>

#include <Rocket/Core/Factory.h>

#include "EventWrapper.h"
#include "EventInterface.h"
#include "EventListenerInstancer.h"

namespace Rocket {
namespace Core {
namespace Python {

static PyObject* py_event = NULL;

void EventInterface::InitialisePythonInterface()
{
	// Define the event.
	py_event = python::class_< Event, EventWrapper, boost::noncopyable >("event", python::init< const char*, Rocket::Core::Dictionary&, bool >())
		.add_property("type", python::make_function( &Event::GetType, python::return_value_policy< python::return_by_value >()))
		.add_property("target_element", python::make_function(&Event::GetTargetElement, python::return_value_policy< python::return_by_value >()))
		.add_property("current_element", python::make_function(&Event::GetCurrentElement, python::return_value_policy< python::return_by_value >()))
		.add_property("parameters", python::make_function(&EventInterface::GetParameters, python::return_value_policy< python::return_by_value >()))
		.def("StopPropagation", &Event::StopPropagation)
		.ptr();

	Rocket::Core::Python::ConverterScriptObject< Event >();
}

void EventInterface::InitialiseRocketInterface()
{
	// Redefine the event instancer.
	Factory::RegisterEventInstancer(new EventInstancer(py_event))->RemoveReference();
	Factory::RegisterEventListenerInstancer(new EventListenerInstancer())->RemoveReference();
}

const Rocket::Core::Dictionary& EventInterface::GetParameters(EventWrapper* self)
{
	return *self->GetParameters();
}

}
}
}
