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

#ifndef ROCKETCORESTYLESHEET_H
#define ROCKETCORESTYLESHEET_H

#include <Rocket/Core/Dictionary.h>
#include <Rocket/Core/ReferenceCountable.h>
#include <set>
#include <Rocket/Core/PropertyDictionary.h>

namespace Rocket {
namespace Core {

class Element;
class ElementDefinition;
class StyleSheetNode;

/**
	StyleSheet maintains a single stylesheet definition. A stylesheet can be combined with another stylesheet to create
	a new, merged stylesheet.

	@author Lloyd Weehuizen
 */

class StyleSheet : public ReferenceCountable
{
public:
	typedef std::set< StyleSheetNode* > NodeList;
	typedef std::map< String, NodeList > NodeIndex;

	StyleSheet();
	virtual ~StyleSheet();

	/// Loads a style from a CSS definition.
	bool LoadStyleSheet(Stream* stream);

	/// Combines this style sheet with another one, producing a new sheet.
	StyleSheet* CombineStyleSheet(const StyleSheet* sheet) const;
	/// Builds the node index for a combined style sheet.
	void BuildNodeIndex();

	/// Returns the compiled element definition for a given element hierarchy. A reference count will be added for the
	/// caller, so another should not be added. The definition should be released by removing the reference count.
	ElementDefinition* GetElementDefinition(const Element* element) const;

protected:
	/// Destroys the style sheet.
	virtual void OnReferenceDeactivate();

private:
	// Root level node, attributes from special nodes like "body" get added to this node
	StyleSheetNode* root;

	// The maximum specificity offset used in this style sheet to distinguish between properties in
	// similarly-specific rules, but declared on different lines. When style sheets are merged, the
	// more-specific style sheet (ie, coming further 'down' the include path) adds the offset of
	// the less-specific style sheet onto its offset, thereby ensuring its properties take
	// precedence in the event of a conflict.
	int specificity_offset;

	// Map of only nodes with actual style information.
	NodeIndex styled_node_index;
	// Map of every node, even empty, un-styled, nodes.
	NodeIndex complete_node_index;

	typedef std::map< String, ElementDefinition* > ElementDefinitionCache;
	// Index of element addresses to element definitions.
	mutable ElementDefinitionCache address_cache;
	// Index of node sets to element definitions.
	mutable ElementDefinitionCache node_cache;
};

}
}

#endif
