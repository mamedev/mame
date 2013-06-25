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

#ifndef ROCKETCOREELEMENTDEFINITION_H
#define ROCKETCOREELEMENTDEFINITION_H

#include <Rocket/Core/Dictionary.h>
#include <Rocket/Core/ReferenceCountable.h>
#include <map>
#include <set>
#include <Rocket/Core/FontEffect.h>
#include "StyleSheetNode.h"

namespace Rocket {
namespace Core {

class Decorator;
class FontEffect;

// Defines for the optimised version of the pseudo-class properties (note the difference from the
// PseudoClassPropertyMap defined in StyleSheetNode.h ... bit clumsy). Here the properties are stored as a list
// of definitions against each property name in specificity-order, along with the pseudo-class requirements for each
// one. This makes it much more straight-forward to query at run-time.
typedef std::pair< StringList, Property > PseudoClassProperty;
typedef std::vector< PseudoClassProperty > PseudoClassPropertyList;
typedef std::map< String, PseudoClassPropertyList > PseudoClassPropertyDictionary;

typedef std::map< String, Decorator* > DecoratorMap;
typedef std::map< StringList, DecoratorMap > PseudoClassDecoratorMap;

/**
	@author Peter Curry
 */

class ElementDefinition : public ReferenceCountable
{
public:
	enum PseudoClassVolatility
	{
		STABLE,					// pseudo-class has no volatility
		FONT_VOLATILE,			// pseudo-class may impact on font effects
		STRUCTURE_VOLATILE		// pseudo-class may impact on definitions of child elements
	};

	ElementDefinition();
	virtual ~ElementDefinition();

	/// Initialises the element definition from a list of style sheet nodes.
	void Initialise(const std::vector< const StyleSheetNode* >& style_sheet_nodes, const PseudoClassList& volatile_pseudo_classes, bool structurally_volatile);

	/// Returns a specific property from the element definition's base properties.
	/// @param[in] name The name of the property to return.
	/// @param[in] pseudo_classes The pseudo-classes currently active on the calling element.
	/// @return The property defined against the give name, or NULL if no such property was found.
	const Property* GetProperty(const String& name, const PseudoClassList& pseudo_classes) const;

	/// Returns the list of properties this element definition defines for an element with the given set of
	/// pseudo-classes.
	/// @param[out] property_names The list to store the defined properties in.
	/// @param[in] pseudo_classes The pseudo-classes defined on the querying element.
	void GetDefinedProperties(PropertyNameList& property_names, const PseudoClassList& pseudo_classes) const;
	/// Returns the list of properties this element definition has explicit definitions for involving the given
	/// pseudo-class.
	/// @param[out] property_names The list of store the newly defined / undefined properties in.
	/// @param[in] pseudo_classes The list of pseudo-classes currently set on the element (post-change).
	/// @param[in] pseudo_class The pseudo-class that was just activated or deactivated.
	void GetDefinedProperties(PropertyNameList& property_names, const PseudoClassList& pseudo_classes, const String& pseudo_class) const;

	/// Iterates over the properties in the definition.
	/// @param[inout] index Index of the property to fetch. This is incremented to the next valid index after the fetch.
	/// @param[in] pseudo_classes The pseudo-classes defined on the querying element.
	/// @param[out] property_pseudo_classes The pseudo-classes the property is defined by.
	/// @param[out] property_name The name of the property at the specified index.
	/// @param[out] property The property at the specified index.
	/// @return True if a property was successfully fetched.
	bool IterateProperties(int& index, const PseudoClassList& pseudo_classes, PseudoClassList& property_pseudo_classes, String& property_name, const Property*& property) const;

	/// Returns the list of the element definition's instanced decorators in the default state.
	/// @return The list of instanced decorators.
	const DecoratorMap& GetDecorators() const;
	/// Returns the map of pseudo-class names to overriding instanced decorators.
	/// @return The map of the overriding decorators for each pseudo-class.
	const PseudoClassDecoratorMap& GetPseudoClassDecorators() const;

	/// Appends this definition's font effects (appropriately for the given pseudo classes) into a
	/// provided map of effects.
	/// @param[out] font_effects The outgoing map of font effects.
	/// @param[in] pseudo_classes Pseudo-classes active on the querying element.
	void GetFontEffects(FontEffectMap& font_effects, const PseudoClassList& pseudo_classes) const;

	/// Returns the volatility of a pseudo-class.
	/// @param[in] pseudo_class The name of the pseudo-class to check for volatility.
	/// @return The volatility of the pseudo-class.
	PseudoClassVolatility GetPseudoClassVolatility(const String& pseudo_class) const;

	/// Returns true if this definition is built from nodes using structural selectors, and therefore is reliant on
	/// siblings remaining stable.
	/// @return True if this definition is structurally volatile.
	bool IsStructurallyVolatile() const;

protected:
	/// Destroys the definition.
	void OnReferenceDeactivate();

private:
	typedef std::pair< String, PropertyDictionary > PropertyGroup;
	typedef std::map< String, PropertyGroup > PropertyGroupMap;

	typedef std::vector< std::pair< StringList, int > > PseudoClassFontEffectIndex;
	typedef std::map< String, PseudoClassFontEffectIndex > FontEffectIndex;

	typedef std::map< String, PseudoClassVolatility > PseudoClassVolatilityMap;

	// Finds all propery declarations for a group.
	void BuildPropertyGroup(PropertyGroupMap& groups, const String& group_type, const PropertyDictionary& element_properties, const PropertyGroupMap* default_properties = NULL);
	// Updates a property dictionary of all properties for a single group.
	int BuildPropertyGroupDictionary(PropertyDictionary& group_properties, const String& group_type, const String& group_name, const PropertyDictionary& element_properties);

	// Builds decorator definitions from the parsed properties and instances decorators as
	// appropriate.
	void InstanceDecorators(const PseudoClassPropertyMap& merged_pseudo_class_properties);
	// Attempts to instance a decorator.
	bool InstanceDecorator(const String& name, const String& type, const PropertyDictionary& properties, const StringList& pseudo_class = StringList());

	// Builds font effect definitions from the parsed properties and instances font effects as
	// appropriate.
	void InstanceFontEffects(const PseudoClassPropertyMap& merged_pseudo_class_properties);
	// Attempts to instance a font effect.
	bool InstanceFontEffect(const String& name, const String& type, const PropertyDictionary& properties, const StringList& pseudo_class = StringList());

	// Returns true if the pseudo-class requirement of a rule is met by a list of an element's pseudo-classes.
	bool IsPseudoClassRuleApplicable(const StringList& rule_pseudo_classes, const PseudoClassList& element_pseudo_classes) const;

	// The attributes for the default state of the element, with no pseudo-classes.
	PropertyDictionary properties;
	// The overridden attributes for the element's pseudo-classes.
	PseudoClassPropertyDictionary pseudo_class_properties;

	// The instanced decorators for this element definition.
	DecoratorMap decorators;
	// The overridden decorators for the element's pseudo-classes.
	PseudoClassDecoratorMap pseudo_class_decorators;

	// The list of every decorator used by this element in every class.
	FontEffectList font_effects;
	// For each unique decorator name, this stores (in order of specificity) the name of the
	// pseudo-class that has a definition for it, and the index into the list of decorators.
	FontEffectIndex font_effect_index;

	// The list of volatile pseudo-classes in this definition, and how volatile they are.
	PseudoClassVolatilityMap pseudo_class_volatility;

	// True if this definition has the potential to change as sibling elements are added or removed.
	bool structurally_volatile;
};

}
}

#endif
