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

#ifndef ROCKETCOREPROPERTYPARSER_H
#define ROCKETCOREPROPERTYPARSER_H

#include <Rocket/Core/Header.h>
#include <Rocket/Core/Property.h>
#include <map>

namespace Rocket {
namespace Core {

typedef std::map< String, int, StringUtilities::StringComparei > ParameterMap;

/**
	A property parser takes a property declaration in string form, validates it, and converts it to a Property.

	@author Peter Curry
 */

class ROCKETCORE_API PropertyParser
{
public:
	virtual ~PropertyParser()
	{
	}

	/// Called to parse a RCSS declaration.
	/// @param[out] property The property to set the parsed value on.
	/// @param[in] value The raw value defined for this property.
	/// @param[in] parameters The list of parameters defined for this property.
	/// @return True if the value was parsed successfully, false otherwise.
	virtual bool ParseValue(Property& property, const String& value, const ParameterMap& parameters) const = 0;

	/// Called when the parser is released. This should free all dynamic memory used by the parser.
	virtual void Release() = 0;
};

}
}

#endif
