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
#include "StyleSheetNode.h"
#include <algorithm>
#include <Rocket/Core/Element.h>
#include "StyleSheetFactory.h"
#include "StyleSheetNodeSelector.h"

namespace Rocket {
namespace Core {

StyleSheetNode::StyleSheetNode(const String& name, NodeType _type, StyleSheetNode* _parent) : name(name)
{
	type = _type;
	parent = _parent;

	specificity = CalculateSpecificity();

	selector = NULL;
	a = 0;
	b = 0;
}

// Constructs a structural style-sheet node.
StyleSheetNode::StyleSheetNode(const String& name, StyleSheetNode* _parent, StyleSheetNodeSelector* _selector, int _a, int _b) : name(name)
{
	type = STRUCTURAL_PSEUDO_CLASS;
	parent = _parent;

	specificity = CalculateSpecificity();

	selector = _selector;
	a = _a;
	b = _b;
}

StyleSheetNode::~StyleSheetNode()
{
	for (int i = 0; i < NUM_NODE_TYPES; i++)
	{
		for (NodeMap::iterator j = children[i].begin(); j != children[i].end(); j++)
			delete (*j).second;
	}
}

// Writes the style sheet node (and all ancestors) into the stream.
void StyleSheetNode::Write(Stream* stream)
{
	if (properties.GetNumProperties() > 0)
	{
		String rule;
		StyleSheetNode* hierarchy = this;
		while (hierarchy != NULL)
		{
			switch (hierarchy->type)
			{
				case TAG:
					rule = " " + hierarchy->name + rule;
					break;

				case CLASS:
					rule = "." + hierarchy->name + rule;
					break;

				case ID:
					rule = "#" + hierarchy->name + rule;
					break;

				case PSEUDO_CLASS:
					rule = ":" + hierarchy->name + rule;
					break;

				case STRUCTURAL_PSEUDO_CLASS:
					rule = ":" + hierarchy->name + rule;
					break;

				default:
					break;
			}

			hierarchy = hierarchy->parent;
		}

		stream->Write(String(1024, "%s /* specificity: %d */\n", StringUtilities::StripWhitespace(rule).CString(), specificity));
		stream->Write("{\n");

		const Rocket::Core::PropertyMap& property_map = properties.GetProperties();
		for (Rocket::Core::PropertyMap::const_iterator i = property_map.begin(); i != property_map.end(); ++i)
		{
			const String& name = i->first;
			const Rocket::Core::Property& property = i->second;

			stream->Write(String(1024, "\t%s: %s; /* specificity: %d */\n", name.CString(), property.value.Get< String >().CString(), property.specificity));
		}

		stream->Write("}\n\n");
	}

	for (size_t i = 0; i < NUM_NODE_TYPES; ++i)
	{
		for (NodeMap::iterator j = children[i].begin(); j != children[i].end(); ++j)
			(*j).second->Write(stream);
	}
}

// Merges an entire tree hierarchy into our hierarchy.
bool StyleSheetNode::MergeHierarchy(StyleSheetNode* node, int specificity_offset)
{
	// Merge the other node's properties into ours.
	MergeProperties(node->properties, specificity_offset);

	selector = node->selector;
	a = node->a;
	b = node->b;

	for (int i = 0; i < NUM_NODE_TYPES; i++)
	{
		for (NodeMap::iterator iterator = node->children[i].begin(); iterator != node->children[i].end(); iterator++)
		{
			StyleSheetNode* local_node = GetChildNode((*iterator).second->name, (NodeType) i);
			local_node->MergeHierarchy((*iterator).second, specificity_offset);
		}
	}

	return true;
}

// Builds up a style sheet's index recursively.
void StyleSheetNode::BuildIndex(StyleSheet::NodeIndex& styled_index, StyleSheet::NodeIndex& complete_index)
{
	// If this is a tag node, then we insert it into the list of all tag nodes. Makes sense, neh?
	if (type == TAG)
	{
		StyleSheet::NodeIndex::iterator iterator = complete_index.find(name);
		if (iterator == complete_index.end())
			(*complete_index.insert(StyleSheet::NodeIndex::value_type(name, StyleSheet::NodeList())).first).second.insert(this);
		else
			(*iterator).second.insert(this);
	}

	// If we are a styled node (ie, have some style attributes attached), then we insert our closest parent tag node
	// into the list of styled tag nodes.
	if (properties.GetNumProperties() > 0)
	{
		StyleSheetNode* tag_node = this;
		while (tag_node != NULL &&
			   tag_node->type != TAG)
			tag_node = tag_node->parent;

		if (tag_node != NULL)
		{
			StyleSheet::NodeIndex::iterator iterator = styled_index.find(tag_node->name);
			if (iterator == styled_index.end())
				(*styled_index.insert(StyleSheet::NodeIndex::value_type(tag_node->name, StyleSheet::NodeList())).first).second.insert(tag_node);
			else
				(*iterator).second.insert(tag_node);
		}
	}

	for (int i = 0; i < NUM_NODE_TYPES; i++)
	{
		for (NodeMap::iterator j = children[i].begin(); j != children[i].end(); ++j)
			(*j).second->BuildIndex(styled_index, complete_index);
	}
}

// Returns the name of this node.
const String& StyleSheetNode::GetName() const
{
	return name;
}

// Returns the specificity of this node.
int StyleSheetNode::GetSpecificity() const
{
	return specificity;
}

// Imports properties from a single rule definition (ie, with a shared specificity) into the node's
// properties.
void StyleSheetNode::ImportProperties(const PropertyDictionary& _properties, int rule_specificity)
{
	properties.Import(_properties, specificity + rule_specificity);
}

// Merges properties from another node (ie, with potentially differing specificities) into the
// node's properties.
void StyleSheetNode::MergeProperties(const PropertyDictionary& _properties, int rule_specificity_offset)
{
	properties.Merge(_properties, rule_specificity_offset);
}

// Returns the node's default properties.
const PropertyDictionary& StyleSheetNode::GetProperties() const
{
	return properties;
}

// Builds the properties of all of the pseudo-classes of this style sheet node into a single map.
void StyleSheetNode::GetPseudoClassProperties(PseudoClassPropertyMap& pseudo_class_properties) const
{
	for (NodeMap::const_iterator i = children[PSEUDO_CLASS].begin(); i != children[PSEUDO_CLASS].end(); ++i)
		(*i).second->GetPseudoClassProperties(pseudo_class_properties, StringList());
}

// Adds to a list the names of this node's pseudo-classes which are deemed volatile.
bool StyleSheetNode::GetVolatilePseudoClasses(PseudoClassList& volatile_pseudo_classes) const
{
	if (type == PSEUDO_CLASS)
	{
		bool self_volatile = !children[TAG].empty();

		for (NodeMap::const_iterator i = children[PSEUDO_CLASS].begin(); i != children[PSEUDO_CLASS].end(); ++i)
			self_volatile = (*i).second->GetVolatilePseudoClasses(volatile_pseudo_classes) | self_volatile;

		if (self_volatile)
			volatile_pseudo_classes.insert(name);

		return self_volatile;
	}
	else
	{
		for (NodeMap::const_iterator i = children[PSEUDO_CLASS].begin(); i != children[PSEUDO_CLASS].end(); ++i)
			(*i).second->GetVolatilePseudoClasses(volatile_pseudo_classes);
	}

	return false;
}

// Returns a direct child node of this node of the requested type.
StyleSheetNode* StyleSheetNode::GetChildNode(const String& child_name, NodeType child_type, bool create)
{
	// Look for a node with given name.
	NodeMap::iterator iterator = children[child_type].find(child_name);
	if (iterator != children[child_type].end())
	{
		// Traverse into node.
		return (*iterator).second;
	}
	else
	{
		if (create)
		{
			StyleSheetNode* new_node = NULL;

			// Create the node; structural pseudo-classes require a little extra leg-work.
			if (child_type == STRUCTURAL_PSEUDO_CLASS)
				new_node = CreateStructuralChild(child_name);
			else
				new_node = new StyleSheetNode(child_name, child_type, this);

			if (new_node != NULL)
			{
				children[child_type][child_name] = new_node;
				return new_node;
			}
		}

		return NULL;
	}
}

// Returns true if this node is applicable to the given element, given its IDs, classes and heritage.
bool StyleSheetNode::IsApplicable(const Element* element) const
{
	// This function is called with an element that matches a style node only with the tag name. We have to determine
	// here whether or not it also matches the required hierarchy.

	// We must have a parent; if not, something's amok with the style tree.
	if (parent == NULL)
	{
		ROCKET_ERRORMSG("Invalid RCSS hierarchy.");
		return false;
	}

	// If we've hit a child of the root of the style sheet tree, then we're done; no more lineage to resolve.
	if (parent->type == ROOT)
		return true;

	// Determine the tag (and possibly id / class as well) of the next required parent in the RCSS hierarchy.
	const StyleSheetNode* parent_node = parent;
	String ancestor_id;
	StringList ancestor_classes;
	StringList ancestor_pseudo_classes;
	std::vector< const StyleSheetNode* > ancestor_structural_pseudo_classes;

	while (parent_node != NULL && parent_node->type != TAG)
	{
		switch (parent_node->type)
		{
			case ID:						ancestor_id = parent_node->name; break;
			case CLASS:						ancestor_classes.push_back(parent_node->name); break;
			case PSEUDO_CLASS:				ancestor_pseudo_classes.push_back(parent_node->name); break;
			case STRUCTURAL_PSEUDO_CLASS:	ancestor_structural_pseudo_classes.push_back(parent_node); break;
			default:						ROCKET_ERRORMSG("Invalid RCSS hierarchy."); return false;
		}

		parent_node = parent_node->parent;
	}

	// Check for an invalid RCSS hierarchy.
	if (parent_node == NULL)
	{
		ROCKET_ERRORMSG("Invalid RCSS hierarchy.");
		return false;
	}

	// Now we know the name / class / ID / pseudo-class / structural requirements for the next ancestor requirement of
	// the element. So we look back through the element's ancestors to find one that matches.
	for (const Element* ancestor_element = element->GetParentNode(); ancestor_element != NULL; ancestor_element = ancestor_element->GetParentNode())
	{
		// Skip this ancestor if the name of the next style node doesn't match its tag name, and one was specified.
		if (!parent_node->name.Empty() 
			&& parent_node->name != ancestor_element->GetTagName())
			continue;

		// Skip this ancestor if the ID of the next style node doesn't match its ID, and one was specified.
		if (!ancestor_id.Empty() &&
			ancestor_id != ancestor_element->GetId())
			continue;

		// Skip this ancestor if the class of the next style node don't match its classes.
		bool resolved_requirements = true;
		for (size_t i = 0; i < ancestor_classes.size(); ++i)
		{
			if (!ancestor_element->IsClassSet(ancestor_classes[i]))
			{
				resolved_requirements = false;
				break;
			}
		}
		if (!resolved_requirements)
			continue;

		// Skip this ancestor if the required pseudo-classes of the style node aren't set on it.
		resolved_requirements = true;
		for (size_t i = 0; i < ancestor_pseudo_classes.size(); ++i)
		{
			if (!ancestor_element->IsPseudoClassSet(ancestor_pseudo_classes[i]))
			{
				resolved_requirements = false;
				break;
			}
		}
		if (!resolved_requirements)
			continue;

		// Skip this ancestor if the required structural pseudo-classes of the style node aren't applicable to it.
		resolved_requirements = true;
		for (size_t i = 0; i < ancestor_structural_pseudo_classes.size(); ++i)
		{
			if (!ancestor_structural_pseudo_classes[i]->selector->IsApplicable(ancestor_element, ancestor_structural_pseudo_classes[i]->a, ancestor_structural_pseudo_classes[i]->b))
			{
				resolved_requirements = false;
				break;
			}
		}
		if (!resolved_requirements)
			continue;

		return parent_node->IsApplicable(ancestor_element);
	}

	// We hit the end of the hierarchy before matching the required ancestor, so bail.
	return false;
}

// Appends all applicable non-tag descendants of this node into the given element list.
void StyleSheetNode::GetApplicableDescendants(std::vector< const StyleSheetNode* >& applicable_nodes, const Element* element) const
{
	// Check if this node matches this element.
	switch (type)
	{
		case ROOT:
		case TAG:
		{
			// These nodes always match.
		}
		break;

		case CLASS:
		{
			if (!element->IsClassSet(name))
				return;
		}
		break;

		case ID:
		{
			if (name != element->GetId())
				return;
		}
		break;

		case PSEUDO_CLASS:
		{
			if (!element->IsPseudoClassSet(name))
				return;
		}
		break;

		case STRUCTURAL_PSEUDO_CLASS:
		{
			if (selector == NULL)
				return;

			if (!selector->IsApplicable(element, a, b))
				return;
		}
		break;
		
		default:
		break;
	}

	if (properties.GetNumProperties() > 0 ||
		!children[PSEUDO_CLASS].empty())
		applicable_nodes.push_back(this);

	for (int i = CLASS; i < NUM_NODE_TYPES; i++)
	{
		// Don't recurse into pseudo-classes; they can't be built into the root definition.
		if (i == PSEUDO_CLASS)
			continue;

		for (NodeMap::const_iterator j = children[i].begin(); j != children[i].end(); ++j)
			(*j).second->GetApplicableDescendants(applicable_nodes, element);
	}
}

// Returns true if this node employs a structural selector, and therefore generates element definitions that are
// sensitive to sibling changes.
bool StyleSheetNode::IsStructurallyVolatile(bool check_ancestors) const
{
	if (type == STRUCTURAL_PSEUDO_CLASS)
		return true;

	if (!children[STRUCTURAL_PSEUDO_CLASS].empty())
		return true;

	// Check our children for structural pseudo-classes.
	for (int i = 0; i < NUM_NODE_TYPES; ++i)
	{
		if (i == STRUCTURAL_PSEUDO_CLASS)
			continue;

		for (NodeMap::const_iterator j = children[i].begin(); j != children[i].end(); ++j)
		{
			if ((*j).second->IsStructurallyVolatile(false))
				return true;
		}
	}

	if (check_ancestors)
	{
		StyleSheetNode* ancestor = parent;
		while (ancestor != NULL)
		{
			if (ancestor->type == STRUCTURAL_PSEUDO_CLASS)
				return true;

			ancestor = ancestor->parent;
		}
	}

	return false;
}

// Constructs a structural pseudo-class child node.
StyleSheetNode* StyleSheetNode::CreateStructuralChild(const String& child_name)
{
	StyleSheetNodeSelector* child_selector = StyleSheetFactory::GetSelector(child_name);
	if (child_selector == NULL)
		return NULL;

	// Parse the 'a' and 'b' values.
	int child_a = 1;
	int child_b = 0;

	size_t parameter_start = child_name.Find("(");
	size_t parameter_end = child_name.Find(")");
	if (parameter_start != String::npos &&
		parameter_end != String::npos)
	{
		String parameters = child_name.Substring(parameter_start + 1, parameter_end - (parameter_start + 1));

		// Check for 'even' or 'odd' first.
		if (parameters == "even")
		{
			child_a = 2;
			child_b = 0;
		}
		else if (parameters == "odd")
		{
			child_a = 2;
			child_b = 1;
		}
		else
		{
			// Alrighty; we've got an equation in the form of [[+/-]an][(+/-)b]. So, foist up, we split on 'n'.
			size_t n_index = parameters.Find("n");
			if (n_index != String::npos)
			{
				// The equation is 0n + b. So a = 0, and we only have to parse b.
				child_a = 0;
				child_b = atoi(parameters.CString());
			}
			else
			{
				if (n_index == 0)
					child_a = 1;
				else
				{
					String a_parameter = parameters.Substring(0, n_index);
					if (StringUtilities::StripWhitespace(a_parameter) == "-")
						child_a = -1;
					else
						child_a = atoi(a_parameter.CString());
				}

				if (n_index == parameters.Length() - 1)
					child_b = 0;
				else
					child_b = atoi(parameters.Substring(n_index + 1).CString());
			}
		}
	}

	return new StyleSheetNode(child_name, this, child_selector, child_a, child_b);
}

// Recursively builds up a list of all pseudo-classes branching from a single node.
void StyleSheetNode::GetPseudoClassProperties(PseudoClassPropertyMap& pseudo_class_properties, const StringList& ancestor_pseudo_classes)
{
	StringList pseudo_classes(ancestor_pseudo_classes);
	pseudo_classes.push_back(name);

	if (properties.GetNumProperties() > 0)
		pseudo_class_properties[pseudo_classes] = properties;

	for (NodeMap::const_iterator i = children[PSEUDO_CLASS].begin(); i != children[PSEUDO_CLASS].end(); ++i)
		(*i).second->GetPseudoClassProperties(pseudo_class_properties, pseudo_classes);
}

int StyleSheetNode::CalculateSpecificity()
{
	// Calculate the specificity of just this node; tags are worth 10,000, IDs 1,000,000 and other specifiers (classes
	// and pseudo-classes) 100,000.

	int specificity = 0;
	switch (type)
	{
		case TAG:
		{
			if (!name.Empty())
				specificity = 10000;
		}
		break;

		case CLASS:
		case PSEUDO_CLASS:
		case STRUCTURAL_PSEUDO_CLASS:
		{
			specificity = 100000;
		}
		break;

		case ID:
		{
			specificity = 1000000;
		}
		break;

		default:
		{
			specificity = 0;
		}
		break;
	}

	// Add our parent's specificity onto ours.
	if (parent != NULL)
		specificity += parent->CalculateSpecificity();

	return specificity;
}

}
}
