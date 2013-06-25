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
#include <Rocket/Core/PropertySpecification.h>
#include "PropertyShorthandDefinition.h"
#include <Rocket/Core/Log.h>
#include <Rocket/Core/PropertyDefinition.h>
#include <Rocket/Core/PropertyDictionary.h>
#include <Rocket/Core/StyleSheetSpecification.h>

namespace Rocket {
namespace Core {

PropertySpecification::PropertySpecification()
{
}

PropertySpecification::~PropertySpecification()
{
	for (PropertyMap::iterator iterator = properties.begin(); iterator != properties.end(); ++iterator)
		delete (*iterator).second;

	for (ShorthandMap::iterator iterator = shorthands.begin(); iterator != shorthands.end(); ++iterator)
		delete (*iterator).second;
}

// Registers a property with a new definition.
PropertyDefinition& PropertySpecification::RegisterProperty(const String& property_name, const String& default_value, bool inherited, bool forces_layout)
{
	String lower_case_name = property_name.ToLower();

	// Create the property and validate the default value.
	PropertyDefinition* property_definition = new PropertyDefinition(default_value, inherited, forces_layout);

	// Delete any existing property.
	PropertyMap::iterator iterator = properties.find(lower_case_name);
	if (iterator != properties.end())
		delete (*iterator).second;

	properties[lower_case_name] = property_definition;
	return *property_definition;
}

// Returns a property definition.
const PropertyDefinition* PropertySpecification::GetProperty(const String& property_name) const
{
	PropertyMap::const_iterator iterator = properties.find(property_name);
	if (iterator == properties.end())
		return NULL;

	return (*iterator).second;
}

// Fetches a list of the names of all registered property definitions.
void PropertySpecification::GetRegisteredProperties(PropertyNameList& _properties) const
{
	for (PropertyMap::const_iterator i = properties.begin(); i != properties.end(); ++i)
		_properties.insert((*i).first);
}

// Registers a shorthand property definition.
bool PropertySpecification::RegisterShorthand(const String& shorthand_name, const String& property_names, ShorthandType type)
{
	StringList properties;
	StringUtilities::ExpandString(properties, property_names.ToLower());

	if (properties.empty())
		return false;

	String lower_case_name = shorthand_name.ToLower();

	// Construct the new shorthand definition and resolve its properties.
	PropertyShorthandDefinition* property_shorthand = new PropertyShorthandDefinition();
	for (size_t i = 0; i < properties.size(); i++)
	{
		const PropertyDefinition* property = GetProperty(properties[i]);
		if (property == NULL)
		{
			Log::Message(Log::LT_ERROR, "Shorthand property '%s' was registered with invalid property '%s'.", shorthand_name.CString(), properties[i].CString());
			delete property_shorthand;

			return false;
		}

		property_shorthand->properties.push_back(PropertyShorthandDefinition::PropertyDefinitionList::value_type(properties[i], property));
	}

	if (type == AUTO)
	{
		if (properties.size() == 4 &&
			properties[0].Find("-top") != String::npos &&
			properties[1].Find("-right") != String::npos &&
			properties[2].Find("-bottom") != String::npos &&
			properties[3].Find("-left") != String::npos)
			property_shorthand->type = BOX;
		else
			property_shorthand->type = FALL_THROUGH;
	}
	else
		property_shorthand->type = type;

	shorthands[lower_case_name] = property_shorthand;
	return true;
}

// Returns a shorthand definition.
const PropertyShorthandDefinition* PropertySpecification::GetShorthand(const String& shorthand_name) const
{
	ShorthandMap::const_iterator iterator = shorthands.find(shorthand_name);
	if (iterator == shorthands.end())
		return NULL;

	return (*iterator).second;
}

// Parses a property declaration, setting any parsed and validated properties on the given dictionary.
bool PropertySpecification::ParsePropertyDeclaration(PropertyDictionary& dictionary, const String& property_name, const String& property_value, const String& source_file, int source_line_number) const
{
	String lower_case_name = property_name.ToLower();

	// Attempt to parse as a single property.
	const PropertyDefinition* property_definition = GetProperty(lower_case_name);

	StringList property_values;
	if (!ParsePropertyValues(property_values, property_value, property_definition == NULL) || property_values.size() == 0)
		return false;

	if (property_definition != NULL)
	{
		Property new_property;
		new_property.source = source_file;
		new_property.source_line_number = source_line_number;
		if (property_definition->ParseValue(new_property, property_values[0]))
		{
			dictionary.SetProperty(lower_case_name, new_property);
			return true;
		}

		return false;
	}

	// Try as a shorthand.
	const PropertyShorthandDefinition* shorthand_definition = GetShorthand(lower_case_name);
	if (shorthand_definition != NULL)
	{
		// If this definition is a 'box'-style shorthand (x-top, x-right, x-bottom, x-left, etc) and there are fewer
		// than four values
		if (shorthand_definition->type == BOX &&
			property_values.size() < 4)
		{
			switch (property_values.size())
			{
				// Only one value is defined, so it is parsed onto all four sides.
				case 1:
				{
					for (int i = 0; i < 4; i++)
					{
						Property new_property;
						if (!shorthand_definition->properties[i].second->ParseValue(new_property, property_values[0]))
							return false;

						new_property.source = source_file;
						new_property.source_line_number = source_line_number;
						dictionary.SetProperty(shorthand_definition->properties[i].first, new_property);
					}
				}
				break;

				// Two values are defined, so the first one is parsed onto the top and bottom value, the second onto
				// the left and right.
				case 2:
				{
					// Parse the first value into the top and bottom properties.
					Property new_property;
					new_property.source = source_file;
					new_property.source_line_number = source_line_number;

					if (!shorthand_definition->properties[0].second->ParseValue(new_property, property_values[0]))
						return false;
					dictionary.SetProperty(shorthand_definition->properties[0].first, new_property);

					if (!shorthand_definition->properties[2].second->ParseValue(new_property, property_values[0]))
						return false;
					dictionary.SetProperty(shorthand_definition->properties[2].first, new_property);

					// Parse the second value into the left and right properties.
					if (!shorthand_definition->properties[1].second->ParseValue(new_property, property_values[1]))
						return false;
					dictionary.SetProperty(shorthand_definition->properties[1].first, new_property);

					if (!shorthand_definition->properties[3].second->ParseValue(new_property, property_values[1]))
						return false;
					dictionary.SetProperty(shorthand_definition->properties[3].first, new_property);
				}
				break;

				// Three values are defined, so the first is parsed into the top value, the second onto the left and
				// right, and the third onto the bottom.
				case 3:
				{
					// Parse the first value into the top property.
					Property new_property;
					new_property.source = source_file;
					new_property.source_line_number = source_line_number;

					if (!shorthand_definition->properties[0].second->ParseValue(new_property, property_values[0]))
						return false;
					dictionary.SetProperty(shorthand_definition->properties[0].first, new_property);

					// Parse the second value into the left and right properties.
					if (!shorthand_definition->properties[1].second->ParseValue(new_property, property_values[1]))
						return false;
					dictionary.SetProperty(shorthand_definition->properties[1].first, new_property);

					if (!shorthand_definition->properties[3].second->ParseValue(new_property, property_values[1]))
						return false;
					dictionary.SetProperty(shorthand_definition->properties[3].first, new_property);

					// Parse the third value into the bottom property.
					if (!shorthand_definition->properties[2].second->ParseValue(new_property, property_values[2]))
						return false;
					dictionary.SetProperty(shorthand_definition->properties[2].first, new_property);
				}
				break;

				default:	break;
			}
		}
		else
		{
			size_t value_index = 0;
			size_t property_index = 0;

			for (; value_index < property_values.size() && property_index < shorthand_definition->properties.size(); property_index++)
			{
				Property new_property;
				new_property.source = source_file;
				new_property.source_line_number = source_line_number;

				if (!shorthand_definition->properties[property_index].second->ParseValue(new_property, property_values[value_index]))
				{
					// This definition failed to parse; if we're falling through, try the next property. If there is no
					// next property, then abort!
					if (shorthand_definition->type == FALL_THROUGH)
					{
						if (property_index + 1 < shorthand_definition->properties.size())
							continue;
					}
					return false;
				}

				dictionary.SetProperty(shorthand_definition->properties[property_index].first, new_property);

				// Increment the value index, unless we're replicating the last value and we're up to the last value.
				if (shorthand_definition->type != REPLICATE ||
					value_index < property_values.size() - 1)
					value_index++;
			}
		}

		return true;
	}

	// Can't find it! Store as an unknown string value.
	Property new_property(property_value, Property::UNKNOWN);
	new_property.source = source_file;
	new_property.source_line_number = source_line_number;
	dictionary.SetProperty(lower_case_name, new_property);

	return true;
}

// Sets all undefined properties in the dictionary to their defaults.
void PropertySpecification::SetPropertyDefaults(PropertyDictionary& dictionary) const
{
	for (PropertyMap::const_iterator i = properties.begin(); i != properties.end(); ++i)
	{
		if (dictionary.GetProperty((*i).first) == NULL)
			dictionary.SetProperty((*i).first, *(*i).second->GetDefaultValue());
	}
}

bool PropertySpecification::ParsePropertyValues(StringList& values_list, const String& values, bool split_values) const
{
	String value;

	enum ParseState { VALUE, VALUE_PARENTHESIS, VALUE_QUOTE };
	ParseState state = VALUE;
	int open_parentheses = 0;

	size_t character_index = 0;
	char previous_character = 0;
	while (character_index < values.Length())
	{
		char character = values[character_index];
		character_index++;

		switch (state)
		{
			case VALUE:
			{
				if (character == ';')
				{
					value = StringUtilities::StripWhitespace(value);
					if (value.Length() > 0)
					{
						values_list.push_back(value);
						value.Clear();
					}
				}
				else if (StringUtilities::IsWhitespace(character))
				{
					if (split_values)
					{
						value = StringUtilities::StripWhitespace(value);
						if (value.Length() > 0)
						{
							values_list.push_back(value);
							value.Clear();
						}
					}
					else
						value.Append(character);
				}
				else if (character == '"')
				{
					if (split_values)
					{
						value = StringUtilities::StripWhitespace(value);
						if (value.Length() > 0)
						{
							values_list.push_back(value);
							value.Clear();
						}
						state = VALUE_QUOTE;
					}
					else
					{
						value.Append(' ');
						state = VALUE_QUOTE;
					}
				}
				else if (character == '(')
				{
					open_parentheses = 1;
					value.Append(character);
					state = VALUE_PARENTHESIS;
				}
				else
				{
					value.Append(character);
				}
			}
			break;

			case VALUE_PARENTHESIS:
			{
				if (previous_character == '/')
				{
					if (character == ')' || character == '(')
						value.Append(character);
					else
					{
						value.Append('/');
						value.Append(character);
					}
				}
				else
				{
					if (character == '(')
					{
						open_parentheses++;
						value.Append(character);
					}
					else if (character == ')')
					{
						open_parentheses--;
						value.Append(character);
						if (open_parentheses == 0)
							state = VALUE;
					}
					else if (character != '/')
					{
						value.Append(character);
					}
				}
			}
			break;

			case VALUE_QUOTE:
			{
				if (previous_character == '/')
				{
					if (character == '"')
						value.Append(character);
					else
					{
						value.Append('/');
						value.Append(character);
					}
				}
				else
				{
					if (character == '"')
					{
						if (split_values)
						{
							value = StringUtilities::StripWhitespace(value);
							if (value.Length() > 0)
							{
								values_list.push_back(value);
								value.Clear();
							}
						}
						else
							value.Append(' ');
						state = VALUE;
					}
					else if (character != '/')
					{
						value.Append(character);
					}
				}
			}
		}

		previous_character = character;
	}

	if (state == VALUE)
	{
		value = StringUtilities::StripWhitespace(value);
		if (value.Length() > 0)
			values_list.push_back(value);
	}

	return true;
}

}
}
