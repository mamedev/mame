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
#include "ElementDefinition.h"
#include <Rocket/Core/Decorator.h>
#include <Rocket/Core/Factory.h>
#include <Rocket/Core/FontDatabase.h>
#include <Rocket/Core/Log.h>

namespace Rocket {
namespace Core {

ElementDefinition::ElementDefinition()
{
	structurally_volatile = false;
}

ElementDefinition::~ElementDefinition()
{
	for (DecoratorMap::iterator i = decorators.begin(); i != decorators.end(); ++i)
		(*i).second->RemoveReference();

	for (PseudoClassDecoratorMap::iterator i = pseudo_class_decorators.begin(); i != pseudo_class_decorators.end(); ++i)
	{
		for (DecoratorMap::iterator j = (*i).second.begin(); j != (*i).second.end(); ++j)
		{
			if ((*j).second != NULL)
				(*j).second->RemoveReference();
		}
	}

	for (size_t i = 0; i < font_effects.size(); ++i)
		font_effects[i]->RemoveReference();
}

// Initialises the element definition from a list of style sheet nodes.
void ElementDefinition::Initialise(const std::vector< const StyleSheetNode* >& style_sheet_nodes, const PseudoClassList& volatile_pseudo_classes, bool _structurally_volatile)
{
	// Set the volatile structure flag.
	structurally_volatile = _structurally_volatile;

	// Mark all the volatile pseudo-classes as structurally volatile.
	for (PseudoClassList::const_iterator i = volatile_pseudo_classes.begin(); i != volatile_pseudo_classes.end(); ++i)
		pseudo_class_volatility[*i] = STRUCTURE_VOLATILE;


	// Merge the default (non-pseudo-class) properties.
	for (size_t i = 0; i < style_sheet_nodes.size(); ++i)
		properties.Merge(style_sheet_nodes[i]->GetProperties());


	// Merge the pseudo-class properties.
	PseudoClassPropertyMap merged_pseudo_class_properties;
	for (size_t i = 0; i < style_sheet_nodes.size(); ++i)
	{
		// Merge all the pseudo-classes.
		PseudoClassPropertyMap node_properties;
		style_sheet_nodes[i]->GetPseudoClassProperties(node_properties);
		for (PseudoClassPropertyMap::iterator j = node_properties.begin(); j != node_properties.end(); ++j)
		{
			// Merge the property maps into one uber-map; for the decorators.
			PseudoClassPropertyMap::iterator k = merged_pseudo_class_properties.find((*j).first);
			if (k == merged_pseudo_class_properties.end())
				merged_pseudo_class_properties[(*j).first] = (*j).second;
			else
				(*k).second.Merge((*j).second);

			// Search through all entries in this dictionary; we'll insert each one into our optimised list of
			// pseudo-class properties.
			for (PropertyMap::const_iterator k = (*j).second.GetProperties().begin(); k != (*j).second.GetProperties().end(); ++k)
			{
				const String& property_name = (*k).first;
				const Property& property = (*k).second;

				// Skip this property if its specificity is lower than the base property's, as in
				// this case it will never be used.
				const Property* default_property = properties.GetProperty(property_name);
				if (default_property != NULL &&
					default_property->specificity >= property.specificity)
					continue;

				PseudoClassPropertyDictionary::iterator l = pseudo_class_properties.find(property_name);
				if (l == pseudo_class_properties.end())
					pseudo_class_properties[property_name] = PseudoClassPropertyList(1, PseudoClassProperty((*j).first, property));
				else
				{
					// Find the location to insert this entry in the map, based on property priorities.
					int index = 0;
					while (index < (int) (*l).second.size() &&
						   (*l).second[index].second.specificity > property.specificity)
						index++;

					(*l).second.insert((*l).second.begin() + index, PseudoClassProperty((*j).first, property));
				}
			}
		}
	}

	InstanceDecorators(merged_pseudo_class_properties);
	InstanceFontEffects(merged_pseudo_class_properties);
}

// Returns a specific property from the element definition's base properties.
const Property* ElementDefinition::GetProperty(const String& name, const PseudoClassList& pseudo_classes) const
{
	// Find a pseudo-class override for this property.
	PseudoClassPropertyDictionary::const_iterator property_iterator = pseudo_class_properties.find(name);
	if (property_iterator != pseudo_class_properties.end())
	{
		const PseudoClassPropertyList& property_list = (*property_iterator).second;
		for (size_t i = 0; i < property_list.size(); ++i)
		{
			if (!IsPseudoClassRuleApplicable(property_list[i].first, pseudo_classes))
				continue;

			return &property_list[i].second;
		}
	}

	return properties.GetProperty(name);
}

// Returns the list of properties this element definition defines for an element with the given set of pseudo-classes.
void ElementDefinition::GetDefinedProperties(PropertyNameList& property_names, const PseudoClassList& pseudo_classes) const
{
	for (PropertyMap::const_iterator i = properties.GetProperties().begin(); i != properties.GetProperties().end(); ++i)
		property_names.insert((*i).first);

	for (PseudoClassPropertyDictionary::const_iterator i = pseudo_class_properties.begin(); i != pseudo_class_properties.end(); ++i)
	{
		// If this property is already in the default dictionary, don't bother checking for it here.
		if (property_names.find((*i).first) != property_names.end())
			continue;

		const PseudoClassPropertyList& property_list = (*i).second;

		// Search through all the pseudo-class combinations that have a definition for this property; if the calling
		// element matches at least one of them, then add it to the list.
		bool property_defined = false;
		for (size_t j = 0; j < property_list.size(); ++j)
		{
			if (IsPseudoClassRuleApplicable(property_list[j].first, pseudo_classes))
			{
				property_defined = true;
				break;
			}
		}

		if (property_defined)
			property_names.insert((*i).first);
	}
}

// Returns the list of properties this element definition has explicit definitions for involving the given
// pseudo-class.
void ElementDefinition::GetDefinedProperties(PropertyNameList& property_names, const PseudoClassList& pseudo_classes, const String& pseudo_class) const
{
	for (PseudoClassPropertyDictionary::const_iterator i = pseudo_class_properties.begin(); i != pseudo_class_properties.end(); ++i)
	{
		// If this property has already been found, don't bother checking for it again.
		if (property_names.find((*i).first) != property_names.end())
			continue;

		const PseudoClassPropertyList& property_list = (*i).second;

		bool property_defined = false;
		for (size_t j = 0; j < property_list.size(); ++j)
		{
			bool rule_valid = true;
			bool found_toggled_pseudo_class = false;

			const StringList& rule_pseudo_classes = property_list[j].first;
			for (size_t j = 0; j < rule_pseudo_classes.size(); ++j)
			{
				if (rule_pseudo_classes[j] == pseudo_class)
				{
					found_toggled_pseudo_class = true;
					continue;
				}

				if (pseudo_classes.find(rule_pseudo_classes[j]) == pseudo_classes.end())
				{			
					rule_valid = false;
					break;
				}
			}

			if (rule_valid &&
				found_toggled_pseudo_class)
			{
				property_defined = true;
				break;
			}
		}

		if (property_defined)
			property_names.insert((*i).first);
	}
}

// Iterates over the properties in the definition.
bool ElementDefinition::IterateProperties(int& index, const PseudoClassList& pseudo_classes, PseudoClassList& property_pseudo_classes, String& property_name, const Property*& property) const
{
	if (index < properties.GetNumProperties())
	{
		PropertyMap::const_iterator i = properties.GetProperties().begin();
		for (int count = 0; count < index; ++count)
			++i;

		property_pseudo_classes.clear();
		property_name = (*i).first;
		property = &((*i).second);
		++index;

		return true;
	}

	// Not in the base properties; check for pseudo-class overrides.
	int property_count = properties.GetNumProperties();
	for (PseudoClassPropertyDictionary::const_iterator i = pseudo_class_properties.begin(); i != pseudo_class_properties.end(); ++i)
	{
		// Iterate over each pseudo-class set that has a definition for this property; if we find one that matches our
		// pseudo-class, increment our index counter and either return that property (if we hit the requested index) or
		// continue looking if we're still below it.
		for (size_t j = 0; j < (*i).second.size(); ++j)
		{
			if (IsPseudoClassRuleApplicable((*i).second[j].first, pseudo_classes))
			{
				property_count++;
				if (property_count > index)
				{
					// Copy the list of pseudo-classes.
					property_pseudo_classes.clear();
					for (size_t k = 0; k < (*i).second[j].first.size(); ++k)
						property_pseudo_classes.insert((*i).second[j].first[k]);

					property_name = (*i).first;
					property = &((*i).second[j].second);
					++index;

					return true;
				}
				else
				{
					break;
				}
			}
		}
	}

	return false;
}

// Returns the list of the element definition's instanced decorators in the default state.
const DecoratorMap& ElementDefinition::GetDecorators() const
{
	return decorators;
}

// Returns the map of pseudo-class names to overriding instanced decorators.
const PseudoClassDecoratorMap& ElementDefinition::GetPseudoClassDecorators() const
{
	return pseudo_class_decorators;
}

// Appends this definition's font effects into a provided map of effects.
void ElementDefinition::GetFontEffects(FontEffectMap& applicable_font_effects, const PseudoClassList& pseudo_classes) const
{
	// Check each set of named effects, looking for applicable ones.
	for (FontEffectIndex::const_iterator i = font_effect_index.begin(); i != font_effect_index.end(); ++i)
	{
		// Search through this list, finding the first effect that is valid (depending on
		// pseudo-classes).
		const PseudoClassFontEffectIndex& index = i->second;
		for (size_t j = 0; j < index.size(); ++j)
		{
			if (IsPseudoClassRuleApplicable(index[j].first, pseudo_classes))
			{
				// This is the most specific valid font effect this element has under the name. If
				// the map of effects already has an effect with the same name, the effect with the
				// highest specificity will prevail.
				FontEffect* applicable_effect = font_effects[index[j].second];

				FontEffectMap::iterator map_iterator = applicable_font_effects.find(i->first);
				if (map_iterator == applicable_font_effects.end() ||
					map_iterator->second->GetSpecificity() < applicable_effect->GetSpecificity())
					applicable_font_effects[i->first] = applicable_effect;

				break;
			}
		}
	}
}

// Returns the volatility of a pseudo-class.
ElementDefinition::PseudoClassVolatility ElementDefinition::GetPseudoClassVolatility(const String& pseudo_class) const
{
	PseudoClassVolatilityMap::const_iterator i = pseudo_class_volatility.find(pseudo_class);
	if (i == pseudo_class_volatility.end())
		return STABLE;
	else
		return i->second;
}

// Returns true if this definition is built from nodes using structural selectors.
bool ElementDefinition::IsStructurallyVolatile() const
{
	return structurally_volatile;
}

// Destroys the definition.
void ElementDefinition::OnReferenceDeactivate()
{
	delete this;
}

// Finds all propery declarations for a group.
void ElementDefinition::BuildPropertyGroup(PropertyGroupMap& groups, const String& group_type, const PropertyDictionary& element_properties, const PropertyGroupMap* default_properties)
{
	String property_suffix = "-" + group_type;

	for (PropertyMap::const_iterator property_iterator = element_properties.GetProperties().begin(); property_iterator != element_properties.GetProperties().end(); ++property_iterator)
	{
		const String& property_name = (*property_iterator).first;
		if (property_name.Length() > property_suffix.Length() &&
			strcasecmp(property_name.CString() + (property_name.Length() - property_suffix.Length()), property_suffix.CString()) == 0)
		{
			// We've found a group declaration!
			String group_name = property_name.Substring(0, property_name.Length() - (group_type.Length() + 1));
			String group_class = (*property_iterator).second.value.Get< String >();
			PropertyDictionary* group_properties = NULL;		

			// Check if we have an existing definition by this name; if so, we're only overriding the type.
			PropertyGroupMap::iterator existing_definition = groups.find(group_name);
			if (existing_definition != groups.end())
			{
				(*existing_definition).second.first = group_class;
				group_properties = &(*existing_definition).second.second;
			}
			else
			{
				// Check if we have any default decorator definitions, and if the new decorator has a default. If so,
				// we make a copy of the default properties for the new decorator.
				if (default_properties != NULL)
				{
					PropertyGroupMap::const_iterator default_definition = default_properties->find(group_name);
					if (default_definition != default_properties->end())
						group_properties = &(*groups.insert(PropertyGroupMap::value_type(group_name, PropertyGroup(group_class, (*default_definition).second.second))).first).second.second;
				}

				// If we still haven't got somewhere to put the properties for the new decorator, make a new
				// definition.
				if (group_properties == NULL)
					group_properties = &(*groups.insert(PropertyGroupMap::value_type(group_name, PropertyGroup(group_class, PropertyDictionary()))).first).second.second;
			}

			// Now find all of this decorator's properties.
			BuildPropertyGroupDictionary(*group_properties, group_type, group_name, element_properties);
		}
	}

	// Now go through all the default decorator definitions and see if the new property list redefines any properties
	// used by them.
	if (default_properties != NULL)
	{
		for (PropertyGroupMap::const_iterator default_definition_iterator = default_properties->begin(); default_definition_iterator != default_properties->end(); ++default_definition_iterator)
		{
			const String& default_definition_name = (*default_definition_iterator).first;

			// Check the list of new definitions hasn't defined this decorator already; if so, it overrode the
			// decorator type and so has inherited all the properties anyway.
			if (groups.find(default_definition_name) == groups.end())
			{
				// Nope! Make a copy of the decorator's properties and see if the new dictionary overrides any of the
				// properties.
				PropertyDictionary decorator_properties = (*default_definition_iterator).second.second;
				if (BuildPropertyGroupDictionary(decorator_properties, group_type, default_definition_name, element_properties) > 0)
					groups[default_definition_name] = PropertyGroup((*default_definition_iterator).second.first, decorator_properties);
			}
		}
	}
}

// Updates a property dictionary of all properties for a single group.
int ElementDefinition::BuildPropertyGroupDictionary(PropertyDictionary& group_properties, const String& ROCKET_UNUSED(group_type), const String& group_name, const PropertyDictionary& element_properties)
{
	int num_properties = 0;

	for (PropertyMap::const_iterator property_iterator = element_properties.GetProperties().begin(); property_iterator != element_properties.GetProperties().end(); ++property_iterator)
	{
		const String& full_property_name = (*property_iterator).first;
		if (full_property_name.Length() > group_name.Length() + 1 &&
			strncasecmp(full_property_name.CString(), group_name.CString(), group_name.Length()) == 0 &&
			full_property_name[group_name.Length()] == '-')
		{
			String property_name = full_property_name.Substring(group_name.Length() + 1);
//			if (property_name == group_type)
//				continue;

			group_properties.SetProperty(property_name, (*property_iterator).second);
			num_properties++;
		}
	}

	return num_properties;
}

// Builds decorator definitions from the parsed properties and instances decorators as appropriate.
void ElementDefinition::InstanceDecorators(const PseudoClassPropertyMap& merged_pseudo_class_properties)
{
	// Now we have the complete property list, we can compile decorator properties and instance as appropriate.
	PropertyGroupMap decorator_definitions;
	BuildPropertyGroup(decorator_definitions, "decorator", properties);
	for (PropertyGroupMap::iterator i = decorator_definitions.begin(); i != decorator_definitions.end(); ++i)
		InstanceDecorator((*i).first, (*i).second.first, (*i).second.second);

	// Now go through all the pseudo-class properties ...
	for (PseudoClassPropertyMap::const_iterator pseudo_class_iterator = merged_pseudo_class_properties.begin(); pseudo_class_iterator != merged_pseudo_class_properties.end(); ++pseudo_class_iterator)
	{
		PropertyGroupMap pseudo_class_decorator_definitions;
		BuildPropertyGroup(pseudo_class_decorator_definitions, "decorator", (*pseudo_class_iterator).second, &decorator_definitions);
		for (PropertyGroupMap::iterator i = pseudo_class_decorator_definitions.begin(); i != pseudo_class_decorator_definitions.end(); ++i)
			InstanceDecorator((*i).first, (*i).second.first, (*i).second.second, (*pseudo_class_iterator).first);
	}
}

// Attempts to instance a decorator into a given list.
bool ElementDefinition::InstanceDecorator(const String& name, const String& type, const PropertyDictionary& properties, const StringList& pseudo_classes)
{
	Decorator* decorator = Factory::InstanceDecorator(type, properties);
	if (decorator == NULL)
	{
		Log::Message(Log::LT_WARNING, "Failed to instance decorator '%s' of type '%s'.", name.CString(), type.CString());
		return false;
	}

	if (pseudo_classes.empty())
	{
		if (decorator != NULL)
			decorators[name] = decorator;
	}
	else
	{
		PseudoClassDecoratorMap::iterator i = pseudo_class_decorators.find(pseudo_classes);
		if (i == pseudo_class_decorators.end())
		{
			DecoratorMap decorators;
			decorators[name] = decorator;

			pseudo_class_decorators[pseudo_classes] = decorators;
		}
		else
			(*i).second[name] = decorator;
	}

	return true;
}

// Builds font effect definitions from the parsed properties and instances font effects as appropriate.
void ElementDefinition::InstanceFontEffects(const PseudoClassPropertyMap& merged_pseudo_class_properties)
{
	// Now we have the complete property list, we can compile font-effect properties and instance as appropriate.
	PropertyGroupMap font_effect_definitions;
	BuildPropertyGroup(font_effect_definitions, "font-effect", properties);
	for (PropertyGroupMap::iterator i = font_effect_definitions.begin(); i != font_effect_definitions.end(); ++i)
		InstanceFontEffect((*i).first, (*i).second.first, (*i).second.second);

	// Now go through all the pseudo-class properties ...
	for (PseudoClassPropertyMap::const_iterator pseudo_class_iterator = merged_pseudo_class_properties.begin(); pseudo_class_iterator != merged_pseudo_class_properties.end(); ++pseudo_class_iterator)
	{
		PropertyGroupMap pseudo_class_font_effect_definitions;
		BuildPropertyGroup(pseudo_class_font_effect_definitions, "font-effect", (*pseudo_class_iterator).second, &font_effect_definitions);
		for (PropertyGroupMap::iterator i = pseudo_class_font_effect_definitions.begin(); i != pseudo_class_font_effect_definitions.end(); ++i)
			InstanceFontEffect((*i).first, (*i).second.first, (*i).second.second, (*pseudo_class_iterator).first);
	}
}

// Attempts to instance a font effect.
bool ElementDefinition::InstanceFontEffect(const String& name, const String& type, const PropertyDictionary& properties, const StringList& pseudo_classes)
{
	FontEffect* font_effect = FontDatabase::GetFontEffect(type, properties);
	if (font_effect == NULL)
	{
		Log::Message(Log::LT_WARNING, "Failed to instance font effect '%s' of type '%s'.", name.CString(), type.CString());
		return false;
	}

	// Push the instanced effect into the list of effects.
	int effect_index = (int) font_effects.size();
	font_effects.push_back(font_effect);

	// Get a reference to the index list we're adding this effect to.
	PseudoClassFontEffectIndex* index = NULL;
	FontEffectIndex::iterator index_iterator = font_effect_index.find(name);
	if (index_iterator == font_effect_index.end())
	{
		// No others; create a new index for this name.
		index = &(font_effect_index.insert(FontEffectIndex::value_type(name, PseudoClassFontEffectIndex())).first->second);
	}
	else
	{
		index = &(index_iterator->second);
	}

	// Add the new effect into the index.
	PseudoClassFontEffectIndex::iterator insert_iterator;
	for (insert_iterator = index->begin(); insert_iterator != index->end(); ++insert_iterator)
	{
		// Keep iterating until we find an effect whose specificity is below the new effect's. The
		// new effect will be inserted before it in the list.
		if (font_effects[insert_iterator->second]->GetSpecificity() < font_effect->GetSpecificity())
			break;
	}

	index->insert(insert_iterator, PseudoClassFontEffectIndex::value_type(pseudo_classes, effect_index));


	// Mark the effect's pseudo-classes as volatile.
	for (size_t i = 0; i < pseudo_classes.size(); ++i)
	{
		PseudoClassVolatilityMap::const_iterator j = pseudo_class_volatility.find(pseudo_classes[i]);
		if (j == pseudo_class_volatility.end())
			pseudo_class_volatility[pseudo_classes[i]] = FONT_VOLATILE;
	}


	return true;
}

// Returns true if the pseudo-class requirement of a rule is met by a list of an element's pseudo-classes.
bool ElementDefinition::IsPseudoClassRuleApplicable(const StringList& rule_pseudo_classes, const PseudoClassList& element_pseudo_classes) const
{
	for (StringList::size_type i = 0; i < rule_pseudo_classes.size(); ++i)
	{
		if (element_pseudo_classes.find(rule_pseudo_classes[i]) == element_pseudo_classes.end())
			return false;
	}

	return true;
}

}
}
