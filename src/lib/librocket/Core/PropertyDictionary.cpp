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
#include <Rocket/Core/PropertyDictionary.h>

namespace Rocket {
namespace Core {

PropertyDictionary::PropertyDictionary()
{
}

PropertyDictionary::~PropertyDictionary()
{
}

// Sets a property on the dictionary. Any existing property with a similar name will be overwritten.
void PropertyDictionary::SetProperty(const String& name, const Property& property)
{
	properties[name] = property;
}

// Removes a property from the dictionary, if it exists.
void PropertyDictionary::RemoveProperty(const String& name)
{
	properties.erase(name);
}

// Returns the value of the property with the requested name, if one exists.
const Property* PropertyDictionary::GetProperty(const String& name) const
{
	PropertyMap::const_iterator iterator = properties.find(name);
	if (iterator == properties.end())
		return NULL;

	return &(*iterator).second;
}

// Returns the number of properties in the dictionary.
int PropertyDictionary::GetNumProperties() const
{
	return properties.size();
}

// Returns the map of properties in the dictionary.
const PropertyMap& PropertyDictionary::GetProperties() const
{
	return properties;
}

// Imports potentially un-specified properties into the dictionary.
void PropertyDictionary::Import(const PropertyDictionary& property_dictionary, int property_specificity)
{
	for (PropertyMap::const_iterator iterator = property_dictionary.properties.begin(); iterator != property_dictionary.properties.end(); ++iterator)
	{
		const Property& property = iterator->second;
		SetProperty(iterator->first, property, property_specificity > 0 ? property_specificity : property.specificity);
	}
}

// Merges the contents of another fully-specified property dictionary with this one.
void PropertyDictionary::Merge(const PropertyDictionary& property_dictionary, int specificity_offset)
{
	for (PropertyMap::const_iterator iterator = property_dictionary.properties.begin(); iterator != property_dictionary.properties.end(); ++iterator)
	{
		const Property& property = iterator->second;
		SetProperty(iterator->first, property, property.specificity + specificity_offset);
	}
}

// Sets a property on the dictionary and its specificity.
void PropertyDictionary::SetProperty(const String& name, const Rocket::Core::Property& property, int specificity)
{
	PropertyMap::iterator iterator = properties.find(name);
	if (iterator != properties.end() &&
		iterator->second.specificity > specificity)
		return;

	Property& new_property = (properties[name] = property);
	new_property.specificity = specificity;
}

}
}
