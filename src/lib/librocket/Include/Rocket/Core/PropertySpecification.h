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

#ifndef ROCKETCOREPROPERTYSPECIFICATION_H
#define ROCKETCOREPROPERTYSPECIFICATION_H

#include <Rocket/Core/Header.h>
#include <Rocket/Core/Element.h>
#include <Rocket/Core/PropertyDefinition.h>

namespace Rocket {
namespace Core {

class PropertyDictionary;
struct PropertyShorthandDefinition;

/**
	A property specification stores a group of property definitions.

	@author Peter Curry
 */

class ROCKETCORE_API PropertySpecification
{
public:
	enum ShorthandType
	{
		// Normal; properties that fail to parse fall-through to the next until they parse correctly, and any
		// undeclared are not set.
		FALL_THROUGH,
		// A single failed parse will abort, and any undeclared are replicated from the last declared property.
		REPLICATE,
		// For 'padding', 'margin', etc; up four properties are expected.
		BOX,
		// BOX if four properties are shorthanded and they end in '-top', '-right', etc, otherwise FALL_THROUGH.
		AUTO
	};

	PropertySpecification();
	~PropertySpecification();

	/// Registers a property with a new definition.
	/// @param[in] property_name The name to register the new property under.
	/// @param[in] default_value The default value to be used for an element if it has no other definition provided.
	/// @param[in] inherited True if this property is inherited from parent to child, false otherwise.
	/// @param[in] forces_layout True if this property requires its parent to be reformatted if changed.
	/// @return The new property definition, ready to have parsers attached.
	PropertyDefinition& RegisterProperty(const String& property_name, const String& default_value, bool inherited, bool forces_layout);
	/// Returns a property definition.
	/// @param[in] property_name The name of the desired property.
	/// @return The appropriate property definition if it could be found, NULL otherwise.
	const PropertyDefinition* GetProperty(const String& property_name) const;

	/// Fetches a list of the names of all registered property definitions.
	/// @param properties[in] The list to store the property names.
	void GetRegisteredProperties(PropertyNameList& properties) const;

	/// Registers a shorthand property definition.
	/// @param[in] shorthand_name The name to register the new shorthand property under.
	/// @param[in] properties A comma-separated list of the properties this definition is shorthand for. The order in which they are specified here is the order in which the values will be processed.
	/// @param[in] type The type of shorthand to declare.
	/// @param True if all the property names exist, false otherwise.
	bool RegisterShorthand(const String& shorthand_name, const String& property_names, ShorthandType type = AUTO);
	/// Returns a shorthand definition.
	/// @param[in] shorthand_name The name of the desired shorthand.
	/// @return The appropriate shorthand definition if it could be found, NULL otherwise.
	const PropertyShorthandDefinition* GetShorthand(const String& shorthand_name) const;

	/// Parses a property declaration, setting any parsed and validated properties on the given dictionary.
	/// @param dictionary The property dictionary which will hold all declared properties.
	/// @param property_name The name of the declared property.
	/// @param property_value The values the property is being set to.
	/// @return True if all properties were parsed successfully, false otherwise.
	bool ParsePropertyDeclaration(PropertyDictionary& dictionary, const String& property_name, const String& property_value, const String& source_file = "", int source_line_number = 0) const;
	/// Sets all undefined properties in the dictionary to their defaults.
	/// @param dictionary[in] The dictionary to set the default values on.
	void SetPropertyDefaults(PropertyDictionary& dictionary) const;

private:
	typedef std::map< String, PropertyDefinition* > PropertyMap;
	typedef std::map< String, PropertyShorthandDefinition* > ShorthandMap;

	PropertyMap properties;
	ShorthandMap shorthands;

	bool ParsePropertyValues(StringList& values_list, const String& values, bool split_values) const;
};

}
}

#endif
