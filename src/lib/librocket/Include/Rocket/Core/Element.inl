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

// Returns the values of one of this element's properties.
// We can assume the property will exist based on the RCSS inheritance.
template < typename T >
T Element::GetProperty(const String& name)
{
	const Property* property = GetProperty(name);
	ROCKET_ASSERTMSG(property, "Invalid property name.");
	return property->Get< T >();
}

// Sets an attribute on the element.
template< typename T >
void Element::SetAttribute(const String& name, const T& value)
{
	attributes.Set(name, value);
	AttributeNameList changed_attributes;
	changed_attributes.insert(name);

	OnAttributeChange(changed_attributes);
}

// Gets the specified attribute, with default value.
template< typename T >
T Element::GetAttribute(const String& name, const T& default_value) const
{			
	return attributes.Get(name, default_value);
}

// Iterates over the attributes.
template< typename T >
bool Element::IterateAttributes(int& index, String& name, T& value) const
{
	return attributes.Iterate(index, name, value);
}
