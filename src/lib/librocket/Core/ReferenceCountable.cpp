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
#include <Rocket/Core/ReferenceCountable.h>

namespace Rocket {
namespace Core {

static int num_outstanding_objects = 0;

// Constructor.
ReferenceCountable::ReferenceCountable(int initial_count)
{
	reference_count = initial_count;
	++num_outstanding_objects;
}

// Destructor. The reference count must be 0 when this is invoked.
ReferenceCountable::~ReferenceCountable()
{
	ROCKET_ASSERT(reference_count == 0);
	--num_outstanding_objects;
}

// Returns the number of references outstanding against this object.
int ReferenceCountable::GetReferenceCount()
{
	return reference_count;
}

// Adds a reference to the object.
void ReferenceCountable::AddReference()
{	
	reference_count++;
	if (reference_count == 1)
	{
		OnReferenceActivate();
	}
}

// Removes a reference from the object.
void ReferenceCountable::RemoveReference()
{
	ROCKET_ASSERT(reference_count > 0);
	reference_count--;	
	if (reference_count == 0)
	{
		OnReferenceDeactivate();
	}
}

ReferenceCountable& ReferenceCountable::operator=(const ReferenceCountable& /*copy*/)
{
	ROCKET_ERRORMSG("Attempting to copy a reference counted object. This is not advisable.");
	return *this;
}

// If any reference countable objects are still allocated, this function will write a leak report to the log.
void ReferenceCountable::DumpLeakReport()
{
	if (num_outstanding_objects > 0)
		Log::Message(Log::LT_WARNING, "%d %s still allocated.", num_outstanding_objects, num_outstanding_objects == 1 ? "object" : "objects");
}

// A hook method called when the reference count climbs from 0.
void ReferenceCountable::OnReferenceActivate()
{
}

// A hook method called when the reference count drops to 0.
void ReferenceCountable::OnReferenceDeactivate()
{
}

}
}
