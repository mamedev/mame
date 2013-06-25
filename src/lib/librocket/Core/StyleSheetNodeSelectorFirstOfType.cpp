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
#include "StyleSheetNodeSelectorFirstOfType.h"
#include <Rocket/Core/Element.h>
#include <Rocket/Core/StyleSheetKeywords.h>

namespace Rocket {
namespace Core {

StyleSheetNodeSelectorFirstOfType::StyleSheetNodeSelectorFirstOfType()
{
}

StyleSheetNodeSelectorFirstOfType::~StyleSheetNodeSelectorFirstOfType()
{
}

// Returns true if the element is the first DOM child in its parent of its type.
bool StyleSheetNodeSelectorFirstOfType::IsApplicable(const Element* element, int ROCKET_UNUSED(a), int ROCKET_UNUSED(b))
{
	Element* parent = element->GetParentNode();
	if (parent == NULL)
		return false;

	int child_index = 0;
	while (child_index < parent->GetNumChildren())
	{
		// If this child is our element, then it's the first one we've found with our tag; the selector succeeds.
		Element* child = parent->GetChild(child_index);
		if (child == element)
			return true;

		// Otherwise, if this child shares our element's tag, then our element is not the first tagged child; the
		// selector fails.
		if (child->GetTagName() == element->GetTagName() &&
			child->GetProperty< int >(DISPLAY) != DISPLAY_NONE)
			return false;

		child_index++;
	}

	return false;
}

}
}
