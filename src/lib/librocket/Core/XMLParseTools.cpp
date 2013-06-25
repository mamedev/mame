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
#include "XMLParseTools.h"
#include <Rocket/Core/StreamMemory.h>
#include "Template.h"
#include "TemplateCache.h"
#include <Rocket/Core.h>
#include <ctype.h>

namespace Rocket {
namespace Core {

// Searchs a string for the specified tag
// NOTE: tag *MUST* be in lowercase
const char* XMLParseTools::FindTag(const char* tag, const char* string, bool closing_tag)
{
	int length = strlen(tag);
	const char* ptr = string;
	bool found_closing = false;

	while (*ptr)
	{
		// Check if the first character matches
		if (tolower((*ptr)) == tag[0])
		{
			// If it does, check the whole word
			if (strncasecmp(ptr,tag,length) == 0)
			{
				// Check for opening <, loop back in the string skipping white space and forward slashes if
				// we're looking for the closing tag
				const char* tag_start = ptr - 1;
				while (tag_start > string && (StringUtilities::IsWhitespace(*tag_start) || *tag_start == '/'))
				{
					if (*tag_start == '/')
						found_closing = true;
					tag_start--;
				}

				// If the character we're looking at is a <, and found closing matches closing tag,
				// its the tag we're looking for
				if (*tag_start == '<' && found_closing == closing_tag)
					return tag_start;

				// Otherwise, keep looking
			}
		}
		ptr++;
	}

	return NULL;
}

bool XMLParseTools::ReadAttribute(const char* &string, String& name, String& value)
{		
	const char* ptr = string;

	name = "";
	value = "";

	// Skip whitespace
	while (StringUtilities::IsWhitespace(*ptr))
		ptr++;

	// Look for the end of the attribute name
	bool found_whitespace = false;
	while (*ptr != '=' && *ptr != '>' && (!found_whitespace || StringUtilities::IsWhitespace(*ptr)))
	{
		if (StringUtilities::IsWhitespace(*ptr))
			found_whitespace = true;
		else
			name += *ptr;	
		ptr++;
	}
	if (*ptr == '>')
		return false;
	
	// If we stopped on an equals, parse the value
	if (*ptr == '=')
	{

		// Skip over white space, ='s and quotes
		bool quoted = false;
		while (StringUtilities::IsWhitespace(*ptr) || *ptr == '\'' || *ptr == '"' || *ptr == '=')
		{
			if (*ptr == '\'' || *ptr == '"')
				quoted = true;
			ptr++;
		}
		if (*ptr == '>')
			return false;

		// Store the value
		while (*ptr != '\'' && *ptr != '"' && *ptr != '>' && (*ptr != ' ' || quoted))
		{
			value += *ptr++;
		}	
		if (*ptr == '>')
			return false;

		// Advance passed the quote
		if (quoted)
			ptr++;
	}
	else
	{
		ptr--;
	}

	// Update the string pointer
	string = ptr;

	return true;
}

Element* XMLParseTools::ParseTemplate(Element* element, const String& template_name)
{	
	// Load the template, and parse it
	Template* parse_template = TemplateCache::GetTemplate(template_name);
	if (!parse_template)
	{
		Log::ParseError(element->GetOwnerDocument()->GetSourceURL(), -1, "Failed to find template '%s'.", template_name.CString());
		return element;
	}

	return parse_template->ParseTemplate(element);
}

}
}
