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

#ifndef ROCKETCOREBASEXMLPARSER_H
#define ROCKETCOREBASEXMLPARSER_H

#include <Rocket/Core/Header.h>
#include <Rocket/Core/Types.h>
#include <Rocket/Core/Dictionary.h>
#include <set>

namespace Rocket {
namespace Core {

class Stream;

typedef Dictionary XMLAttributes;

/**
	@author Peter Curry
 */

class ROCKETCORE_API BaseXMLParser
{
	public:
		BaseXMLParser();
		virtual ~BaseXMLParser();

		/// Registers a tag as containing general character data. This will mean the contents of the tag will be parsed
		/// similarly to a CDATA tag (ie, no other markup will be recognised until the section's closing tag is found).
		/// @param[in] tag The tag to register as containing generic character data.
		void RegisterCDATATag(const String& tag);

		/// Parses the given stream as an XML file, and calls the handlers when
		/// interesting phenomena are encountered.
		void Parse(Stream* stream);

		/// Get the line number in the stream.
		/// @return The line currently being processed in the XML stream.
		int GetLineNumber();

		/// Called when the parser finds the beginning of an element tag.
		virtual void HandleElementStart(const String& name, const XMLAttributes& attributes);
		/// Called when the parser finds the end of an element tag.
		virtual void HandleElementEnd(const String& name);
		/// Called when the parser encounters data.
		virtual void HandleData(const String& data);

	protected:
		// The stream we're reading the XML from.
		Stream* xml_source;

	private:
		void ReadHeader();
		void ReadBody();

		bool ReadOpenTag();
		bool ReadCloseTag();
		bool ReadAttributes(XMLAttributes& attributes);
		bool ReadCDATA(const char* terminator = NULL);

		// Reads from the stream until a complete word is found.
		// @param[out] word Word thats been found
		// @param[in] terminators List of characters that terminate the search
		bool FindWord(String& word, const char* terminators = NULL);
		// Reads from the stream until the given character set is found. All
		// intervening characters will be returned in data.
		bool FindString(const unsigned char* string, String& data);
		// Returns true if the next sequence of characters in the stream
		// matches the given string. If consume is set and this returns true,
		// the characters will be consumed.
		bool PeekString(const unsigned char* string, bool consume = true);

		// Fill the buffer as much as possible, without removing any content that is still pending
		bool FillBuffer();

		unsigned char* read;
		unsigned char* buffer;
		int buffer_size;
		int buffer_used;
		int line_number;
		int open_tag_depth;

		// The element attributes being read.
		XMLAttributes attributes;
		// The loose data being read.
		String data;

		std::set< String > cdata_tags;
};

}
}

#endif
