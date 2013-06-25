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

#ifndef ROCKETCOREPROPERTYDICTIONARY_H
#define ROCKETCOREPROPERTYDICTIONARY_H

#include <Rocket/Core/Header.h>
#include <Rocket/Core/Property.h>

namespace Rocket {
namespace Core {

typedef std::map< String, Property > PropertyMap;

/**
	A dictionary to property names to values.

	@author Peter Curry
 */

class ROCKETCORE_API PropertyDictionary
{
public:
	PropertyDictionary();
	~PropertyDictionary();

	/// Sets a property on the dictionary. Any existing property with a similar name will be overwritten.
	/// @param[in] name The name of the property to add.
	/// @param[in] property The value of the new property.
	void SetProperty(const String& name, const Property& property);
	/// Removes a property from the dictionary, if it exists.
	/// @param[in] name The name of the property to remove.
	void RemoveProperty(const String& name);
	/// Returns the value of the property with the requested name, if one exists.
	/// @param[in] name The name of the desired property.
	const Property* GetProperty(const String& name) const;

	/// Returns the number of properties in the dictionary.
	/// @return The number of properties in the dictionary.
	int GetNumProperties() const;
	/// Returns the map of properties in the dictionary.
	/// @return The property map.
	const PropertyMap& GetProperties() const;

	/// Imports into the dictionary, and optionally defines the specificity of, potentially
	/// un-specified properties. In the case of name conflicts, the incoming properties will
	/// overwrite the existing properties if their specificity (or their forced specificity)
	/// are at least equal.
	/// @param[in] property_dictionary The properties to import.
	/// @param[in] specificity The specificity for all incoming properties. If this is not specified, the properties will keep their original specificity.
	void Import(const PropertyDictionary& property_dictionary, int specificity = -1);

	/// Merges the contents of another fully-specified property dictionary with this one.
	/// Properties defined in the new dictionary will overwrite those with the same name as
	/// appropriate.
	/// @param[in] property_dictionary The dictionary to merge.
	/// @param[in] specificity_offset The specificities of all incoming properties will be offset by this value.
	void Merge(const PropertyDictionary& property_dictionary, int specificity_offset = 0);

private:
	// Sets a property on the dictionary and its specificity if there is no name conflict, or its
	// specificity (given by the parameter, not read from the property itself) is at least equal to
	// the specificity of the conflicting property.
	void SetProperty(const String& name, const Rocket::Core::Property& property, int specificity);

	PropertyMap properties;
};

}
}

#endif
