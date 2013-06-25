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

#ifndef ROCKETCORESTYLESHEETSPECIFICATION_H
#define ROCKETCORESTYLESHEETSPECIFICATION_H

#include <Rocket/Core/Header.h>
#include <Rocket/Core/PropertySpecification.h>
#include <Rocket/Core/Types.h>

namespace Rocket {
namespace Core {

class PropertyParser;

/**
	@author Peter Curry
 */

class ROCKETCORE_API StyleSheetSpecification
{
public:
	/// Starts up the specification structure and registers default properties and type parsers.
	/// @return True if the specification started up successfully, false if not.
	static bool Initialise();
	/// Destroys the specification structure and releases the parsers.
	static void Shutdown();

	/// Registers a parser for use in property definitions.
	/// @param[in] parser_name The name to register the new parser under.
	/// @param[in] parser The parser to register. This parser will be released by the specification.
	/// @return True if the parser was registered successfully, false otherwise.
	static bool RegisterParser(const String& parser_name, PropertyParser* parser);
	/// Returns the parser registered with a specific name.
	/// @param[in] parser_name The name of the desired parser.
	/// @return The parser registered under the given name, or NULL if no such parser exists.
	static PropertyParser* GetParser(const String& parser_name);

	/// Registers a property with a new definition.
	/// @param[in] property_name The name to register the new property under.
	/// @param[in] default_value The default value to be used for an element if it has no other definition provided.
	/// @param[in] inherited True if this property is inherited from parent to child, false otherwise.
	/// @param[in] forces_layout True if a change in this property on an element will cause the element's layout to possibly change.
	/// @return The new property definition, ready to have parsers attached.
	static PropertyDefinition& RegisterProperty(const String& property_name, const String& default_value, bool inherited, bool forces_layout = false);
	/// Returns a property definition.
	/// @param[in] property_name The name of the desired property.
	/// @return The appropriate property definition if it could be found, NULL otherwise.
	static const PropertyDefinition* GetProperty(const String& property_name);

	/// Fetches a list of the names of all registered property definitions.
	/// @param properties[in] The list to store the property names.
	static void GetRegisteredProperties(PropertyNameList& properties);

	/// Registers a shorthand property definition.
	/// @param[in] shorthand_name The name to register the new shorthand property under.
	/// @param[in] properties A comma-separated list of the properties this definition is shorthand for. The order in which they are specified here is the order in which the values will be processed.
	/// @param[in] type The type of shorthand to declare.
	/// @param True if all the property names exist, false otherwise.
	static bool RegisterShorthand(const String& shorthand_name, const String& property_names, PropertySpecification::ShorthandType type = PropertySpecification::AUTO);
	/// Returns a shorthand definition.
	/// @param[in] shorthand_name The name of the desired shorthand.
	/// @return The appropriate shorthand definition if it could be found, NULL otherwise.
	static const PropertyShorthandDefinition* GetShorthand(const String& shorthand_name);

	/// Parses a property declaration, setting any parsed and validated properties on the given dictionary.
	/// @param[in] dictionary The property dictionary which will hold all declared properties.
	/// @param[in] property_name The name of the declared property.
	/// @param[in] property_value The values the property is being set to.
	/// @param[in] source_file The file where this property was declared. Used for error reporting, debugging and relative paths for referenced assets.
	/// @param[in] line_number The location of the source file where this property was declared. Used for error reporting and debugging.
	/// @return True if all properties were parsed successfully, false otherwise.
	static bool ParsePropertyDeclaration(PropertyDictionary& dictionary, const String& property_name, const String& property_value, const String& source_file = "", int source_line_number = 0);

private:
	StyleSheetSpecification();
	~StyleSheetSpecification();

	// Registers Rocket's default parsers.
	void RegisterDefaultParsers();
	// Registers Rocket's default style properties.
	void RegisterDefaultProperties();

	// Parsers used by all property definitions.
	typedef std::map< String, PropertyParser* > ParserMap;
	ParserMap parsers;

	// The properties defined in the style sheet specification.
	PropertySpecification properties;
};

}
}

#endif
