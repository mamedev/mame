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
#include <Rocket/Core/BaseXMLParser.h>

namespace Rocket {
namespace Core {

// Most file layers cache 4k.
const int DEFAULT_BUFFER_SIZE = 4096;

BaseXMLParser::BaseXMLParser()
{
	read = NULL;
	buffer = NULL;
	buffer_used = 0;
	buffer_size = 0;
	open_tag_depth = 0;
}

BaseXMLParser::~BaseXMLParser()
{
}

// Registers a tag as containing general character data.
void BaseXMLParser::RegisterCDATATag(const String& tag)
{
	if (!tag.Empty())
		cdata_tags.insert(tag.ToLower());
}

// Parses the given stream as an XML file, and calls the handlers when
// interesting phenomenon are encountered.
void BaseXMLParser::Parse(Stream* stream)
{
	xml_source = stream;
	buffer_size = DEFAULT_BUFFER_SIZE;

	buffer = (unsigned char*) malloc(buffer_size);
	read = buffer;
	line_number = 1;
	FillBuffer();

	// Read (er ... skip) the header, if one exists.
	ReadHeader();
	// Read the XML body.
	ReadBody();

	free(buffer);
}

// Get the current file line number
int BaseXMLParser::GetLineNumber()
{
	return line_number;
}

// Called when the parser finds the beginning of an element tag.
void BaseXMLParser::HandleElementStart(const String& ROCKET_UNUSED(name), const XMLAttributes& ROCKET_UNUSED(attributes))
{
}

// Called when the parser finds the end of an element tag.
void BaseXMLParser::HandleElementEnd(const String& ROCKET_UNUSED(name))
{
}

// Called when the parser encounters data.
void BaseXMLParser::HandleData(const String& ROCKET_UNUSED(data))
{
}

void BaseXMLParser::ReadHeader()
{
	if (PeekString((unsigned char*) "<?"))
	{
		String temp;
		FindString((unsigned char*) ">", temp);
	}
}

void BaseXMLParser::ReadBody()
{
	open_tag_depth = 0;

	for(;;)
	{
		// Find the next open tag.
		if (!FindString((unsigned char*) "<", data))
			break;

		// Check what kind of tag this is.
		if (PeekString((const unsigned char*) "!--"))
		{
			// Comment.
			String temp;
			if (!FindString((const unsigned char*) "-->", temp))
				break;
		}
		else if (PeekString((const unsigned char*) "![CDATA["))
		{
			// CDATA tag; read everything (including markup) until the ending
			// CDATA tag.
			if (!ReadCDATA())
				break;
		}
		else if (PeekString((const unsigned char*) "/"))
		{
			if (!ReadCloseTag())
				break;

			// Bail if we've hit the end of the XML data.
			if (open_tag_depth == 0)
			{
				xml_source->Seek((read - buffer) - buffer_used, SEEK_CUR);
				break;
			}
		}
		else
		{
			if (!ReadOpenTag())
				break;
		}
	}

	// Check for error conditions
	if (open_tag_depth > 0)
	{
		Log::Message(Log::LT_WARNING, "XML parse error on line %d of %s.", GetLineNumber(), xml_source->GetSourceURL().GetURL().CString());
	}
}

bool BaseXMLParser::ReadOpenTag()
{
	// Increase the open depth
	open_tag_depth++;

	// Opening tag; send data immediately and open the tag.
	if (!data.Empty())
	{
		HandleData(data);
		data.Clear();
	}

	String tag_name;
	if (!FindWord(tag_name, "/>"))
		return false;

	bool section_opened = false;

	if (PeekString((const unsigned char*) ">"))
	{
		// Simple open tag.
		HandleElementStart(tag_name, XMLAttributes());
		section_opened = true;
	}
	else if (PeekString((const unsigned char*) "/") &&
			 PeekString((const unsigned char*) ">"))
	{
		// Empty open tag.
		HandleElementStart(tag_name, XMLAttributes());
		HandleElementEnd(tag_name);

		// Tag immediately closed, reduce count
		open_tag_depth--;
	}
	else
	{
		// It appears we have some attributes. Let's parse them.
		XMLAttributes attributes;
		if (!ReadAttributes(attributes))
			return false;

		if (PeekString((const unsigned char*) ">"))
		{
			HandleElementStart(tag_name, attributes);
			section_opened = true;
		}
		else if (PeekString((const unsigned char*) "/") &&
				 PeekString((const unsigned char*) ">"))
		{
			HandleElementStart(tag_name, attributes);
			HandleElementEnd(tag_name);

			// Tag immediately closed, reduce count
			open_tag_depth--;
		}
		else
		{
			return false;
		}
	}

	// Check if this tag needs to processed as CDATA.
	if (section_opened)
	{
		String lcase_tag_name = tag_name.ToLower();
		if (cdata_tags.find(lcase_tag_name) != cdata_tags.end())
		{
			if (ReadCDATA(lcase_tag_name.CString()))
			{
				open_tag_depth--;
				if (!data.Empty())
				{
					HandleData(data);
					data.Clear();
				}
				HandleElementEnd(tag_name);

				return true;
			}

			return false;
		}
	}

	return true;
}

bool BaseXMLParser::ReadCloseTag()
{
	// Closing tag; send data immediately and close the tag.
	if (!data.Empty())
	{
		HandleData(data);
		data.Clear();
	}

	String tag_name;
	if (!FindString((const unsigned char*) ">", tag_name))
		return false;

	HandleElementEnd(StringUtilities::StripWhitespace(tag_name));

	// Tag closed, reduce count
	open_tag_depth--;

	return true;
}

bool BaseXMLParser::ReadAttributes(XMLAttributes& attributes)
{
	for (;;)
	{
		String attribute;
		String value;

		// Get the attribute name		
		if (!FindWord(attribute, "=/>"))
		{			
			return false;
		}
		
		// Check if theres an assigned value
		if (PeekString((const unsigned char*)"="))
		{
			if (PeekString((const unsigned char*) "\""))
			{
				if (!FindString((const unsigned char*) "\"", value))
					return false;
			}
			else if (PeekString((const unsigned char*) "'"))
			{
				if (!FindString((const unsigned char*) "'", value))
					return false;
			}
			else if (!FindWord(value, "/>"))
			{
				return false;
			}
		}

 		attributes.Set(attribute.CString(), value);

		// Check for the end of the tag.
		if (PeekString((const unsigned char*) "/", false) ||
			PeekString((const unsigned char*) ">", false))
			return true;
	}
}

bool BaseXMLParser::ReadCDATA(const char* terminator)
{
	String cdata;
	if (terminator == NULL)
	{
		FindString((const unsigned char*) "]]>", cdata);
		data += cdata;
		return true;
	}
	else
	{
		for (;;)
		{
			// Search for the next tag opening.
			if (!FindString((const unsigned char*) "<", cdata))
				return false;

			if (PeekString((const unsigned char*) "/", false))
			{
				String tag;
				if (FindString((const unsigned char*) ">", tag))
				{
					String tag_name = StringUtilities::StripWhitespace(tag.Substring(tag.Find("/") + 1));
					if (tag_name.ToLower() == terminator)
					{
						data += cdata;
						return true;
					}
					else
					{
						cdata += "<";
						cdata += tag;
						cdata += ">";
					}
				}
				else
					cdata += "<";
			}
			else
				cdata += "<";
		}
	}
}

// Reads from the stream until a complete word is found.
bool BaseXMLParser::FindWord(String& word, const char* terminators)
{
	for (;;)
	{
		if (read >= buffer + buffer_used)
		{
			if (!FillBuffer())			
				return false;
		}

		// Ignore white space
		if (StringUtilities::IsWhitespace(*read))
		{
			if (word.Empty())
			{
				read++;
				continue;
			}
			else
				return true;
		}

		// Check for termination condition
		if (terminators && strchr(terminators, *read))
		{
			return !word.Empty();
		}

		word += *read;
		read++;
	}
}

// Reads from the stream until the given character set is found.
bool BaseXMLParser::FindString(const unsigned char* string, String& data)
{
	int index = 0;
	while (string[index])
	{
		if (read >= buffer + buffer_used)
		{
			if (!FillBuffer())
				return false;
		}

		// Count line numbers
		if (*read == '\n')
		{
			line_number++;
		}

		if (*read == string[index])
		{
			index += 1;
		}
		else
		{
			if (index > 0)
			{
				data.Append((const char*) string, index);
				index = 0;
			}

			data += *read;
		}

		read++;
	}

	return true;
}

// Returns true if the next sequence of characters in the stream matches the
// given string.
bool BaseXMLParser::PeekString(const unsigned char* string, bool consume)
{
	unsigned char* peek_read = read;

	int i = 0;
	while (string[i])
	{
		// If we're about to read past the end of the buffer, read into the
		// overflow buffer.
		if ((peek_read - buffer) + i >= buffer_used)
		{
			int peek_offset = peek_read - read;
			FillBuffer();
			peek_read = read + peek_offset;

			if (peek_read - buffer + i >= buffer_used)
			{
				// Wierd, seems our buffer is too small, realloc it bigger.
				buffer_size *= 2;
				int read_offset = read - buffer;
				buffer = (unsigned char*) realloc(buffer, buffer_size);

				// Restore the read pointers.
				read = buffer + read_offset;
				peek_read = read + peek_offset;
				
				// Attempt to fill our new buffer size.
				if (!FillBuffer())
					return false;
			}
		}

		// Seek past all the whitespace if we haven't hit the initial character yet.
		if (i == 0 && StringUtilities::IsWhitespace(*peek_read))
		{
			peek_read++;
		}
		else
		{
			if (*peek_read != string[i])
				return false;

			i++;
			peek_read++;
		}
	}

	// Set the read pointer to the end of the peek.
	if (consume)
	{		
		read = peek_read;
	}

	return true;
}

// Fill the buffer as much as possible, without removing any content that is still pending
bool BaseXMLParser::FillBuffer()
{
	int bytes_free = buffer_size;
	int bytes_remaining = Math::Max((int)(buffer_used - (read - buffer)), 0);

	// If theres any data still in the buffer, shift it down, and fill it again
	if (bytes_remaining > 0)
	{
		memmove(buffer, read, bytes_remaining);
		bytes_free = buffer_size - bytes_remaining;
	}
	
	read = buffer;
	size_t bytes_read = xml_source->Read(&buffer[bytes_remaining], bytes_free);
	buffer_used = bytes_read + bytes_remaining;

	return bytes_read > 0;
}

}
}
