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

#ifndef ROCKETCOREREFERENCECOUNTABLE_H
#define ROCKETCOREREFERENCECOUNTABLE_H

#include <Rocket/Core/Header.h>

namespace Rocket {
namespace Core {

/**
	A base class for any class that wishes to be reference counted.
	@author Robert Curry
*/

class ROCKETCORE_API ReferenceCountable
{
public:
	/// Constructor.
	/// @param[in] initial_count The initial reference count of the object.
	ReferenceCountable(int initial_count = 1);
	/// Destructor. The reference count must be 0 when this is invoked.
	virtual ~ReferenceCountable();

	/// Returns the number of references outstanding against this object.
	virtual int GetReferenceCount();
	/// Increases the reference count. If this pushes the count above 0, OnReferenceActivate() will be called. 
	virtual void AddReference();
	/// Decreases the reference count. If this pushes the count to 0, OnReferenceDeactivate() will be called. 
	virtual void RemoveReference();

	/// Catches incorrect copy attempts.
	ReferenceCountable& operator=(const ReferenceCountable& copy);

	/// If any reference countable objects are still allocated, this function will write a leak report to the log.
	static void DumpLeakReport();

protected:		
	/// A hook method called when the reference count climbs from 0.
	virtual void OnReferenceActivate();
	/// A hook method called when the reference count drops to 0.
	virtual void OnReferenceDeactivate();

private:
	// The number of references against this object.
	int reference_count;
};

}
}

#endif
