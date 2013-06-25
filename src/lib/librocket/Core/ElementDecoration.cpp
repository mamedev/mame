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
#include "ElementDecoration.h"
#include "ElementDefinition.h"
#include <Rocket/Core/Decorator.h>
#include <Rocket/Core/Element.h>

namespace Rocket {
namespace Core {

ElementDecoration::ElementDecoration(Element* _element)
{
	element = _element;
	active_decorators_dirty = false;
}

ElementDecoration::~ElementDecoration()
{
	ReleaseDecorators();
}

// Releases existing decorators and loads all decorators required by the element's definition.
bool ElementDecoration::ReloadDecorators()
{
	ReleaseDecorators();

	const ElementDefinition* definition = element->GetDefinition();
	if (definition == NULL)
		return true;

	// Generate the decorator sets for pseudo-classes with overrides.
	const PseudoClassDecoratorMap& pseudo_class_decorators = definition->GetPseudoClassDecorators();
	for (PseudoClassDecoratorMap::const_iterator i = pseudo_class_decorators.begin(); i != pseudo_class_decorators.end(); ++i)
	{
		for (DecoratorMap::const_iterator j = (*i).second.begin(); j != (*i).second.end(); ++j)
		{
			int index = LoadDecorator((*j).second);

			// Add it into the index. If a decorator with the same name already exists for this element, then we add it
			// into the list at the right position (sorted by specificity, descending).
			PseudoClassDecoratorIndexList* pseudo_class_decorator_index = NULL;
			DecoratorIndex::iterator index_iterator = decorator_index.find((*j).first);
			if (index_iterator == decorator_index.end())
				pseudo_class_decorator_index = &(*decorator_index.insert(DecoratorIndex::value_type((*j).first, PseudoClassDecoratorIndexList())).first).second;
			else
				pseudo_class_decorator_index = &(*index_iterator).second;

			// Add the decorator index at the right point to maintain the order of the list.
			PseudoClassDecoratorIndexList::iterator k = pseudo_class_decorator_index->begin();
			for (; k != pseudo_class_decorator_index->end(); ++k)
			{
				if (decorators[(*k).second].decorator->GetSpecificity() < decorators[index].decorator->GetSpecificity())
					break;
			}

			pseudo_class_decorator_index->insert(k, PseudoClassDecoratorIndex(PseudoClassList((*i).first.begin(), (*i).first.end()), index));
		}
	}

	// Put the decorators for the element's default state at the end of any index lists.
	const DecoratorMap& default_decorators = definition->GetDecorators();
	for (DecoratorMap::const_iterator i = default_decorators.begin(); i != default_decorators.end(); ++i)
	{
		int index = LoadDecorator((*i).second);

		DecoratorIndex::iterator index_iterator = decorator_index.find((*i).first);
		if (index_iterator == decorator_index.end())
			decorator_index.insert(DecoratorIndex::value_type((*i).first, PseudoClassDecoratorIndexList(1, PseudoClassDecoratorIndex(PseudoClassList(), index))));
		else
			(*index_iterator).second.push_back(PseudoClassDecoratorIndex(PseudoClassList(), index));
	}

	active_decorators_dirty = true;

	return true;
}

// Loads a single decorator and adds it to the list of loaded decorators for this element.
int ElementDecoration::LoadDecorator(Decorator* decorator)
{
	DecoratorHandle element_decorator;
	element_decorator.decorator = decorator;
	element_decorator.decorator->AddReference();
	element_decorator.decorator_data = decorator->GenerateElementData(element);

	decorators.push_back(element_decorator);
	return (int) (decorators.size() - 1);
}

// Releases all existing decorators and frees their data.
void ElementDecoration::ReleaseDecorators()
{
	for (size_t i = 0; i < decorators.size(); i++)
	{
		if (decorators[i].decorator_data)
			decorators[i].decorator->ReleaseElementData(decorators[i].decorator_data);

		decorators[i].decorator->RemoveReference();
	}

	decorators.clear();
	active_decorators.clear();
	decorator_index.clear();
}

// Updates the list of active decorators (if necessary).
void ElementDecoration::UpdateActiveDecorators()
{
	if (active_decorators_dirty)
	{
		active_decorators.clear();

		for (DecoratorIndex::iterator i = decorator_index.begin(); i != decorator_index.end(); ++i)
		{
			PseudoClassDecoratorIndexList& indices = (*i).second;
			for (size_t j = 0; j < indices.size(); ++j)
			{
				if (element->ArePseudoClassesSet(indices[j].first))
				{
					// Insert the new index into the list of active decorators, ordered by z-index.
					float z_index = decorators[indices[j].second].decorator->GetZIndex();
					std::vector< int >::iterator insert_iterator = active_decorators.begin();
					while (insert_iterator != active_decorators.end() &&
						   z_index > decorators[(*insert_iterator)].decorator->GetZIndex())
						++insert_iterator;

					active_decorators.insert(insert_iterator, indices[j].second);

					break;
				}
			}
		}

		active_decorators_dirty = false;
	}
}

void ElementDecoration::RenderDecorators()
{
	UpdateActiveDecorators();

	// Render the decorators attached to this element in its current state.
	for (size_t i = 0; i < active_decorators.size(); i++)
	{
		DecoratorHandle& decorator = decorators[active_decorators[i]];
		decorator.decorator->RenderElement(element, decorator.decorator_data);
	}
}

void ElementDecoration::DirtyDecorators()
{
	active_decorators_dirty = true;
}

// Iterates over all active decorators attached to the decoration's element.
bool ElementDecoration::IterateDecorators(int& index, PseudoClassList& pseudo_classes, String& name, Decorator*& decorator, DecoratorDataHandle& decorator_data) const
{
	if (index < 0)
		return false;

	size_t count = 0;

	for (DecoratorIndex::const_iterator index_iterator = decorator_index.begin(); index_iterator != decorator_index.end(); ++index_iterator)
	{
		// This is the list of all pseudo-classes that have a decorator under this name.
		const PseudoClassDecoratorIndexList& decorator_index_list = index_iterator->second;
		if (count + decorator_index_list.size() <= (size_t) index)
		{
			count += decorator_index_list.size();
			continue;
		}

		// This is the one we're looking for.
		name = index_iterator->first;

		int relative_index = index - count;
		pseudo_classes = decorator_index_list[relative_index].first;

		const DecoratorHandle& decorator_handle = decorators[decorator_index_list[relative_index].second];
		decorator = decorator_handle.decorator;
		decorator_data = decorator_handle.decorator_data;

		index += 1;
		return true;
	}

	return false;
}

}
}
