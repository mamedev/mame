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

#ifndef ROCKETCORESTYLESHEETNODE_H
#define ROCKETCORESTYLESHEETNODE_H

#include <Rocket/Core/PropertyDictionary.h>
#include <Rocket/Core/StyleSheet.h>
#include <Rocket/Core/Types.h>

namespace Rocket {
namespace Core {

class StyleSheetNodeSelector;

typedef std::map< StringList, PropertyDictionary > PseudoClassPropertyMap;

/**
	A style sheet is composed of a tree of nodes.

	@author Pete / Lloyd
 */

class StyleSheetNode
{
public:
	enum NodeType
	{
		TAG = 0,
		CLASS,
		ID,
		PSEUDO_CLASS,
		STRUCTURAL_PSEUDO_CLASS,
		NUM_NODE_TYPES,	// only counts the listed node types
		ROOT			// special node type we don't keep in a list
	};

	/// Constructs a generic style-sheet node.
	StyleSheetNode(const String& name, NodeType type, StyleSheetNode* parent = NULL);
	/// Constructs a structural style-sheet node.
	StyleSheetNode(const String& name, StyleSheetNode* parent, StyleSheetNodeSelector* selector, int a, int b);
	~StyleSheetNode();

	/// Writes the style sheet node (and all ancestors) into the stream.
	void Write(Stream* stream);

	/// Merges an entire tree hierarchy into our hierarchy.
	bool MergeHierarchy(StyleSheetNode* node, int specificity_offset = 0);
	/// Builds up a style sheet's index recursively.
	void BuildIndex(StyleSheet::NodeIndex& styled_index, StyleSheet::NodeIndex& complete_index);

	/// Returns the name of this node.
	const String& GetName() const;
	/// Returns the specificity of this node.
	int GetSpecificity() const;

	/// Imports properties from a single rule definition into the node's properties and sets the
	/// appropriate specificity on them. Any existing attributes sharing a key with a new attribute
	/// will be overwritten if they are of a lower specificity.
	/// @param[in] properties The properties to import.
	/// @param[in] rule_specificity The specificity of the importing rule.
	void ImportProperties(const PropertyDictionary& properties, int rule_specificity);
	/// Merges properties from another node (ie, with potentially differing specificities) into the
	/// node's properties. Any existing properties sharing a key with a new attribute will be
	/// overwritten if they are of a lower specificity.
	/// @param[in] properties The properties to merge with this node's.
	/// @param[in] rule_specificity_offset The offset of the importing properties' specificities.
	void MergeProperties(const PropertyDictionary& properties, int rule_specificity_offset);
	/// Returns the node's default properties.
	const PropertyDictionary& GetProperties() const;

	/// Merges the properties of all of the pseudo-classes of this style sheet node into a single map.
	/// @param pseudo_class_properties[out] The dictionary to fill with the pseudo-class properties.
	void GetPseudoClassProperties(PseudoClassPropertyMap& pseudo_class_properties) const;
	/// Adds to a list the names of this node's pseudo-classes which are deemed volatile; that is, which will
	/// potentially affect child node's element definition if set or unset.
	/// @param volatile_pseudo_classes[out] The list of volatile pseudo-classes.
	bool GetVolatilePseudoClasses(PseudoClassList& volatile_pseudo_classes) const;

	/// Returns a direct child node of this node of the requested type.
	/// @param name The name of the child node to fetch.
	/// @param type The type of node to fetch; this must be one of either TAG, CLASS, ID, PSEUDO_CLASS or PSEUDO_CLASS_STRUCTURAL.
	/// @param create If set to true, the node will be created if it doesn't exist.
	StyleSheetNode* GetChildNode(const String& name, NodeType type, bool create = true);

	/// Returns true if this node is applicable to the given element, given its IDs, classes and heritage.
	bool IsApplicable(const Element* element) const;
	/// Appends all applicable non-tag descendants of this node into the given element list.
	void GetApplicableDescendants(std::vector< const StyleSheetNode* >& applicable_nodes, const Element* element) const;

	/// Returns true if this node employs a structural selector, and therefore generates element definitions that are
	/// sensitive to sibling changes.
	/// @return True if this node uses a structural selector.
	bool IsStructurallyVolatile(bool check_ancestors = true) const;

private:
	// Constructs a structural pseudo-class child node.
	StyleSheetNode* CreateStructuralChild(const String& child_name);
	// Recursively builds up a list of all pseudo-classes branching from a single node.
	void GetPseudoClassProperties(PseudoClassPropertyMap& pseudo_class_properties, const StringList& ancestor_pseudo_classes);

	int CalculateSpecificity();

	// The parent of this node; is NULL for the root node.
	StyleSheetNode* parent;

	// The name and type.
	String name;
	NodeType type;

	// The complex selector for this node; only used for structural nodes.
	StyleSheetNodeSelector* selector;
	int a;
	int b;

	// A measure of specificity of this node; the attribute in a node with a higher value will override those of a
	// node with a lower value.
	int specificity;

	// The generic properties for this node.
	PropertyDictionary properties;

	// This node's child nodes, whether standard tagged children, or further derivations of this tag by ID or class.
	typedef std::map< String, StyleSheetNode* > NodeMap;
	NodeMap children[NUM_NODE_TYPES];
};

}
}

#endif
