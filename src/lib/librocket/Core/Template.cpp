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
#include "Template.h"
#include "XMLParseTools.h"
#include <Rocket/Core/ElementUtilities.h>
#include <Rocket/Core/XMLParser.h>

namespace Rocket {
namespace Core {

Template::Template()
{
	body = NULL;
}

Template::~Template()
{
	if (body)
		body->RemoveReference();
}

const String& Template::GetName() const
{
	return name;
}

bool Template::Load(Stream* stream)
{
	// Load the entire template into memory so we can pull out
	// the header and body tags
	String buffer;	
	stream->Read(buffer, stream->Length());

	// Pull out the header
	const char* head_start = XMLParseTools::FindTag("head", buffer.CString());
	if (!head_start)	
		return false;

	const char* head_end = XMLParseTools::FindTag("head", head_start, true);
	if (!head_end)	
		return false;
	// Advance to the end of the tag
	head_end = strchr(head_end, '>') + 1;

	// Pull out the body	
	const char* body_start = XMLParseTools::FindTag("body", head_end);
	if (!body_start)	
		return false;
	
	const char* body_end = XMLParseTools::FindTag("body", body_start, true);
	if (!body_end)
		return false;
	// Advance to the end of the tag
	body_end = strchr(body_end, '>') + 1;

	// Find the RML tag, skip over it and read the attributes,
	// storing the ones we're interested in.
	String attribute_name;
	String attribute_value;
	const char* ptr = XMLParseTools::FindTag("template", buffer.CString());
	if (!ptr)
		return false;

	while (XMLParseTools::ReadAttribute(++ptr, attribute_name, attribute_value))
	{
		if (attribute_name == "name")
			name = attribute_value;
		if (attribute_name == "content")
			content = attribute_value;
	}

	// Create a stream around the header, parse it and store it
	StreamMemory* header_stream = new StreamMemory((const byte*) head_start,head_end - head_start);
	header_stream->SetSourceURL(stream->GetSourceURL());

	XMLParser parser(NULL);
	parser.Parse(header_stream);

	header_stream->RemoveReference();

	header = *parser.GetDocumentHeader();

	// Store the body in stream form
	body = new StreamMemory(body_end - body_start);	
	body->SetSourceURL(stream->GetSourceURL());
	body->PushBack(body_start, body_end - body_start);

	return true;
}

Element* Template::ParseTemplate(Element* element)
{
	body->Seek(0, SEEK_SET);

	XMLParser parser(element);
	parser.Parse(body);

	// If theres an inject attribute on the template, 
	// attempt to find the required element
	if (!content.Empty())
	{
		Element* content_element = ElementUtilities::GetElementById(element, content);
		if (content_element)
			element = content_element;
	}

	return element;
}

const DocumentHeader* Template::GetHeader()
{
	return &header;
}

}
}
