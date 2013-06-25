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

#ifndef ROCKETCORESTYLESHEETPARSER_H
#define ROCKETCORESTYLESHEETPARSER_H

namespace Rocket {
namespace Core {

class PropertyDictionary;
class Stream;
class StyleSheetNode;

/**
	Helper class for parsing a style sheet into its memory representation.

	@author Lloyd Weehuizen
 */

class StyleSheetParser
{
public:
	StyleSheetParser();
	~StyleSheetParser();

	/// Parses the given stream into the style sheet
	/// @param node The root node the stream will be parsed into
	/// @param stream The stream to read
	/// @return The number of parsed rules, or -1 if an error occured.
	int Parse(StyleSheetNode* node, Stream* stream);	

	/// Parses the given string into the property dictionary
	/// @param parsed_properties The properties dictionary the properties will be read into
	/// @param properties The properties to parse
	/// @return True if the parse was successful, or false if an error occured.
	bool ParseProperties(PropertyDictionary& parsed_properties, const String& properties);

private:
	// Stream we're parsing from.
	Stream* stream;
	// Parser memory buffer.
	String parse_buffer;
	// How far we've read through the buffer.
	size_t parse_buffer_pos;

	// The name of the file we'r parsing.
	String stream_file_name;
	// Current line number we're parsing.
	size_t line_number;

	// Parses properties from the parse buffer into the dictionary
	// @param properties The dictionary to store the properties in
	bool ReadProperties(PropertyDictionary& properties);

	// Import properties into the stylesheet node
	// @param node Node to import into
	// @param names The names of the nodes
	// @param properties The dictionary of properties
	// @param rule_specificity The specifity of the rule
	bool ImportProperties(StyleSheetNode* node, const String& names, const PropertyDictionary& properties, int rule_specificity);

	// Attempts to find one of the given character tokens in the active stream
	// If it's found, buffer is filled with all content up until the token
	// @param buffer The buffer that receives the content
	// @param characters The character tokens to find
	// @param remove_token If the token that caused the find to stop should be removed from the stream
	bool FindToken(String& buffer, const char* tokens, bool remove_token);

	// Attempts to find the next character in the active stream.
	// If it's found, buffer is filled with the character
	// @param buffer The buffer that receives the character, if read.
	bool ReadCharacter(char& buffer);

	// Fill the internal parse buffer
	bool FillBuffer();
};

}
}

#endif
