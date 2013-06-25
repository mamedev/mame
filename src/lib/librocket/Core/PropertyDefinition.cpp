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
#include <Rocket/Core/PropertyDefinition.h>
#include <Rocket/Core/Log.h>
#include <Rocket/Core/StyleSheetSpecification.h>

namespace Rocket {
namespace Core {

PropertyDefinition::PropertyDefinition(const String& _default_value, bool _inherited, bool _forces_layout) : default_value(_default_value, Property::UNKNOWN)
{
	inherited = _inherited;
	forces_layout = _forces_layout;
}

PropertyDefinition::~PropertyDefinition()
{
}

// Registers a parser to parse values for this definition.
PropertyDefinition& PropertyDefinition::AddParser(const String& parser_name, const String& parser_parameters)
{
	ParserState new_parser;

	// Fetch the parser.
	new_parser.parser = StyleSheetSpecification::GetParser(parser_name);
	if (new_parser.parser == NULL)
	{
		Log::Message(Log::LT_ERROR, "Property was registered with invalid parser '%s'.", parser_name.CString());
		return *this;
	}

	// Split the parameter list, and set up the map.
	if (!parser_parameters.Empty())
	{
		StringList parameter_list;
		StringUtilities::ExpandString(parameter_list, parser_parameters);
		for (size_t i = 0; i < parameter_list.size(); i++)
			new_parser.parameters[parameter_list[i]] = (int) i;
	}

	parsers.push_back(new_parser);

	// If the default value has not been parsed successfully yet, run it through the new parser.
	if (default_value.unit == Property::UNKNOWN)
	{
		String unparsed_value = default_value.value.Get< String >();
		if (!new_parser.parser->ParseValue(default_value, unparsed_value, new_parser.parameters))
		{
			default_value.value.Set(unparsed_value);
			default_value.unit = Property::UNKNOWN;
		}
	}

	return *this;
}

// Called when parsing a RCSS declaration.
bool PropertyDefinition::ParseValue(Property& property, const String& value) const
{
	for (size_t i = 0; i < parsers.size(); i++)
	{
		if (parsers[i].parser->ParseValue(property, value, parsers[i].parameters))
		{
			property.definition = this;
			property.parser_index = (int) i;
			return true;
		}
	}

	property.unit = Property::UNKNOWN;
	return false;
}

// Called to convert a parsed property back into a value.
bool PropertyDefinition::GetValue(String& value, const Property& property) const
{
	value = property.value.Get< String >();

	switch (property.unit)
	{
		case Property::KEYWORD:
		{
			if (property.parser_index < 0 || property.parser_index >= (int) parsers.size())
				return false;

			int keyword = property.value.Get< int >();
			for (ParameterMap::const_iterator i = parsers[property.parser_index].parameters.begin(); i != parsers[property.parser_index].parameters.end(); ++i)
			{
				if ((*i).second == keyword)
				{
					value = (*i).first;
					break;
				}
			}

			return false;
		}
		break;

		case Property::COLOUR:
		{
			Colourb colour = property.value.Get< Colourb >();
			value.FormatString(32, "rgb(%d,%d,%d,%d)", colour.red, colour.green, colour.blue, colour.alpha);
		}
		break;

		case Property::PX:		value.Append("px"); break;
		case Property::EM:		value.Append("em"); break;
		case Property::PERCENT:	value.Append("%"); break;
		default:					break;
	}

	return true;
}

// Returns true if this property is inherited from a parent to child elements.
bool PropertyDefinition::IsInherited() const
{
	return inherited;
}

// Returns true if this property forces a re-layout when changed.
bool PropertyDefinition::IsLayoutForced() const
{
	return forces_layout;
}

// Returns the default for this property.
const Property* PropertyDefinition::GetDefaultValue() const
{
	return &default_value;
}

}
}
