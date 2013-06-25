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
#include "PropertyParserColour.h"

namespace Rocket {
namespace Core {

PropertyParserColour::PropertyParserColour()
{
	html_colours["black"] = Colourb(0, 0, 0);
	html_colours["silver"] = Colourb(192, 192, 192);
	html_colours["gray"] = Colourb(128, 128, 128);
	html_colours["grey"] = Colourb(128, 128, 128);
	html_colours["white"] = Colourb(255, 255, 255);
	html_colours["maroon"] = Colourb(128, 0, 0);
	html_colours["red"] = Colourb(255, 0, 0);
	html_colours["orange"] = Colourb(255, 165, 0);
	html_colours["purple"] = Colourb(128, 0, 128);
	html_colours["fuschia"] =  Colourb(255, 0, 255);
	html_colours["green"] =  Colourb(0, 128, 0);
	html_colours["lime"] =  Colourb(0, 255, 0);
	html_colours["olive"] =  Colourb(128, 128, 0);
	html_colours["yellow"] =  Colourb(255, 255, 0);
	html_colours["navy"] =  Colourb(0, 0, 128);
	html_colours["blue"] =  Colourb(0, 0, 255);
	html_colours["teal"] =  Colourb(0, 128, 128);
	html_colours["aqua"] = Colourb(0, 255, 255);
	html_colours["transparent"] = Colourb(255, 255, 255, 0);
}

PropertyParserColour::~PropertyParserColour()
{
}

// Called to parse a RCSS colour declaration.
bool PropertyParserColour::ParseValue(Property& property, const String& value, const ParameterMap& ROCKET_UNUSED(parameters)) const
{
	if (value.Empty())
		return false;

	Colourb colour;

	// Check for a hex colour.
	if (value[0] == '#')
	{
		char hex_values[4][2] = { {'f', 'f'},
								  {'f', 'f'},
								  {'f', 'f'},
								  {'f', 'f'} };

		switch (value.Length())
		{
			// Single hex digit per channel, RGB and alpha.
			case 5:		hex_values[3][0] = hex_values[3][1] = value[4];

			// Single hex digit per channel, RGB only.
			case 4:		hex_values[0][0] = hex_values[0][1] = value[1];
						hex_values[1][0] = hex_values[1][1] = value[2];
						hex_values[2][0] = hex_values[2][1] = value[3];
						break;

			// Two hex digits per channel, RGB and alpha.
			case 9:		hex_values[3][0] = value[7];
						hex_values[3][1] = value[8];

			// Two hex digits per channel, RGB only.
			case 7:		memcpy(hex_values, &value.CString()[1], sizeof(char) * 6);
						break;

			default:
				return false;
		}

		// Parse each of the colour elements.
		for (int i = 0; i < 4; i++)
		{
			int tens = Math::HexToDecimal(hex_values[i][0]);
			int ones = Math::HexToDecimal(hex_values[i][1]);
			if (tens == -1 ||
				ones == -1)
				return false;

			colour[i] = (byte) (tens * 16 + ones);
		}
	}
	else if (value.Substring(0, 3) == "rgb")
	{
		StringList values;

		int find = value.Find("(") + 1;
		StringUtilities::ExpandString(values, value.Substring(find, value.RFind(")") - find), ',');

		// Check if we're parsing an 'rgba' or 'rgb' colour declaration.
		if (value.Length() > 3 && value[3] == 'a')
		{
			if (values.size() != 4)
				return false;
		}
		else
		{
			if (values.size() != 3)
				return false;

			values.push_back("255");
		}

		// Parse the three RGB values.
		for (int i = 0; i < 4; ++i)
		{
			int component;

			// We're parsing a percentage value.
			if (values[i].Length() > 0 && values[i][values[i].Length() - 1] == '%')
				component = Math::RealToInteger((float) (atof(values[i].Substring(0, values[i].Length() - 1).CString()) / 100.0f) * 255.0f);
			// We're parsing a 0 -> 255 integer value.
			else
				component = atoi(values[i].CString());

			colour[i] = (byte) (Math::Clamp(component, 0, 255));
		}
	}
	else
	{
		// Check for the specification of an HTML colour.
		ColourMap::const_iterator iterator = html_colours.find(value);
		if (iterator == html_colours.end())
			return false;
		else
			colour = (*iterator).second;
	}

	property.value = Variant(colour);
	property.unit = Property::COLOUR;

	return true;
}

// Destroys the parser.
void PropertyParserColour::Release()
{
	delete this;
}

}
}
