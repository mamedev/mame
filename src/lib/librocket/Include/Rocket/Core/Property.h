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

#ifndef ROCKETCOREPROPERTY_H
#define ROCKETCOREPROPERTY_H

#include <Rocket/Core/Variant.h>
#include <Rocket/Core/Header.h>

namespace Rocket {
namespace Core {

class PropertyDefinition;

/**
	@author Peter Curry
 */

class ROCKETCORE_API Property
{
public:
	enum Unit
	{
		UNKNOWN = 1 << 0,

		KEYWORD = 1 << 1,			// generic keyword; fetch as < int >

		STRING = 1 << 2,			// generic string; fetch as < String >

		// Absolute values.
		NUMBER = 1 << 3,			// number unsuffixed; fetch as < float >
		PX = 1 << 4,				// number suffixed by 'px'; fetch as < float >
		COLOUR = 1 << 5,			// colour; fetch as < Colourb >
		ABSOLUTE_UNIT = NUMBER | PX | COLOUR,

		// Relative values.
		EM = 1 << 6,				// number suffixed by 'em'; fetch as < float >
		PERCENT = 1 << 7,			// number suffixed by '%'; fetch as < float >
		RELATIVE_UNIT = EM | PERCENT
	};

	Property();
	template < typename PropertyType >
	Property(PropertyType value, Unit unit, int specificity = -1) : value(Variant(value)), unit(unit), specificity(specificity)
	{
		definition = NULL;
		parser_index = -1;

		source_line_number = 0;
	}

	~Property();	

	/// Get the value of the property as a string.
	String ToString() const;

	/// Templatised accessor.
	template <typename T>
	T Get() const
	{
		return value.Get<T>();
	}

	Variant value;
	Unit unit;
	int specificity;

	const PropertyDefinition* definition;
	int parser_index;

	String source;
	int source_line_number;
};

}
}

#endif
