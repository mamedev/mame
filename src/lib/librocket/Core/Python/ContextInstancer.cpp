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
#include "ContextInstancer.h"
#include <Rocket/Core/Python/Utilities.h>

namespace Rocket {
namespace Core {
namespace Python {

ContextInstancer::ContextInstancer(PyObject* _instancer)
{
	instancer = _instancer;
	Py_INCREF(instancer);
}

ContextInstancer::~ContextInstancer()
{
	Py_DECREF(instancer);
}

// Instances a context.
Context* ContextInstancer::InstanceContext(const Rocket::Core::String& name)
{
	// Put the arguments into a tuple.
	PyObject* args = PyTuple_New(1);	
	PyTuple_SetItem(args, 0, PyString_FromString(name.CString()));

	// Instance the context.
	PyObject* instance = PyObject_CallObject(instancer, args);
	Py_DECREF(args);

	// Rebind the target element.
	Context* context = python::extract< Context* >(Rocket::Core::Python::Utilities::MakeObject(instance));
	if (context != NULL)
	{
		// The wrapper will remove the initial C++ reference (as the object may have been made in Python), so we have
		// to add the reference back again ... and remove the Python reference this will add!
		context->AddReference();
		Py_DECREF(instance);
	}

	return context;
}

// Releases a context previously created by this context.
void ContextInstancer::ReleaseContext(Context* ROCKET_UNUSED(context))
{
}

// Releases this context instancer
void ContextInstancer::Release()
{
	delete this;
}
}
}

}
