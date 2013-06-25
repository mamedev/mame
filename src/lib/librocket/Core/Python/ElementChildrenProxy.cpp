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
#include "ElementChildrenProxy.h"
#include <Rocket/Core/Property.h>
#include <Rocket/Core/Element.h>

namespace Rocket {
namespace Core {
namespace Python {

ElementChildrenProxy::ElementChildrenProxy(Element* _element)
{
	element = _element;
}

ElementChildrenProxy::~ElementChildrenProxy()
{
}

void ElementChildrenProxy::InitialisePythonInterface()
{
	python::class_<ElementChildrenProxy>("childrenproxy", python::no_init)
		.def("__len__", &ElementChildrenProxy::Len )
		.def("__getitem__", &ElementChildrenProxy::GetItem, python::return_value_policy< python::return_by_value >())
		;
}

int ElementChildrenProxy::Len()
{
	return element->GetNumChildren();
}

Element* ElementChildrenProxy::GetItem(int index)
{
	// Support indexing from both ends
	if (index < 0)
	{
		index = element->GetNumChildren() + index;
	}

	// Throw exception if we're out of range, this is required to support python iteration
	if (index >= element->GetNumChildren())
	{
		PyErr_SetString(PyExc_IndexError, "Index out of range");
		python::throw_error_already_set();
	}

	return element->GetChild(index);
}

}
}
}
