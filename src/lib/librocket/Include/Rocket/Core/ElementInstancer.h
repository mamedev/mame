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

#ifndef ROCKETCOREELEMENTINSTANCER_H
#define ROCKETCOREELEMENTINSTANCER_H

#include <Rocket/Core/ReferenceCountable.h>
#include <Rocket/Core/XMLParser.h>
#include <Rocket/Core/Header.h>

namespace Rocket {
namespace Core {

class Element;

/**
	An element instancer provides a method for allocating
	an deallocating elements.

	Node handlers are reference counted, so that the same handler
	can be used for multiple tags.

	It is important at the same instancer that allocated
	the element releases it. This ensures there are no
	issues with memory from different DLLs getting mixed up.

	@author Lloyd Weehuizen
 */ 

class ROCKETCORE_API ElementInstancer : public ReferenceCountable
{
public:
	virtual ~ElementInstancer();

	/// Instances an element given the tag name and attributes.
	/// @param[in] parent The element the new element is destined to be parented to.
	/// @param[in] tag The tag of the element to instance.
	/// @param[in] attributes Dictionary of attributes.
	virtual Element* InstanceElement(Element* parent, const String& tag, const XMLAttributes& attributes) = 0;
	/// Releases an element instanced by this instancer.
	/// @param[in] element The element to release.
	virtual void ReleaseElement(Element* element) = 0;
	/// Release the instancer.
	virtual void Release() = 0;

protected:
	virtual void OnReferenceDeactivate();
};

}
}

#endif
