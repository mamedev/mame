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

#ifndef ROCKETCOREELEMENTDECORATION_H
#define ROCKETCOREELEMENTDECORATION_H

#include <Rocket/Core/Types.h>

namespace Rocket {
namespace Core {

class Decorator;
class Element;

/**
	Manages an elements decorator state

	@author Lloyd Weehuizen
 */

class ElementDecoration
{
public:
	/// Constructor
	/// @param element The element this decorator with acting on
	ElementDecoration(Element* element);
	~ElementDecoration();

	// Releases existing decorators and loads all decorators required by the element's definition.
	bool ReloadDecorators();	

	/// Renders all appropriate decorators.
	void RenderDecorators();

	/// Mark decorators as dirty and force them to reset themselves
	void DirtyDecorators();

	/// Iterates over all active decorators attached to the decoration's element.
	/// @param[inout] index Index to fetch. This is incremented after the fetch.
	/// @param[out] pseudo_classes The pseudo-classes the decorator required to be active before it renders.
	/// @param[out] name The name of the decorator at the specified index.
	/// @param[out] decorator The decorator at the specified index.
	/// @param[out] decorator_data This element's handle to any data is has stored against the decorator.
	/// @return True if a decorator was successfully fetched, false if not.
	bool IterateDecorators(int& index, PseudoClassList& pseudo_classes, String& name, Decorator*& decorator, DecoratorDataHandle& decorator_data) const;

private:
	// Loads a single decorator and adds it to the list of loaded decorators for this element.
	int LoadDecorator(Decorator* decorator);
	// Releases all existing decorators and frees their data.
	void ReleaseDecorators();
	// Updates the list of active decorators (if necessary)
	void UpdateActiveDecorators();

	struct DecoratorHandle
	{
		Decorator* decorator;
		DecoratorDataHandle decorator_data;
	};

	typedef std::vector< DecoratorHandle > DecoratorList;
	typedef std::pair< PseudoClassList, int > PseudoClassDecoratorIndex;
	typedef std::vector< PseudoClassDecoratorIndex > PseudoClassDecoratorIndexList;
	typedef std::map< String, PseudoClassDecoratorIndexList > DecoratorIndex;

	// The element this decorator belongs to
	Element* element;

	// The list of every decorator used by this element in every class.
	DecoratorList decorators;
	// The list of currently active decorators.
	std::vector< int > active_decorators;
	bool active_decorators_dirty;

	// For each unique decorator name, this stores (in order of specificity) the name of the pseudo-class that has
	// a definition for it, and the index into the list of decorators.
	DecoratorIndex decorator_index;
};

}
}

#endif
