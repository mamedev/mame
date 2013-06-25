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
#include "XMLNodeHandlerHead.h"
#include "DocumentHeader.h"
#include "TemplateCache.h"
#include <Rocket/Core.h>

namespace Rocket {
namespace Core {

XMLNodeHandlerHead::XMLNodeHandlerHead()
{
}

XMLNodeHandlerHead::~XMLNodeHandlerHead()
{
}

Element* XMLNodeHandlerHead::ElementStart(XMLParser* parser, const String& name, const XMLAttributes& attributes)
{
	if (name == "head")
	{
		// Process the head attribute
		parser->GetDocumentHeader()->source = parser->GetSourceURL().GetURL();
	}

	// Is it a link tag?
	else if (name == "link")
	{
		// Lookup the type and href
		String type = attributes.Get<String>("type", "").ToLower();
		String href = attributes.Get<String>("href", "");

		if (!type.Empty() && !href.Empty())
		{
			// If its RCSS (... or CSS!), add to the RCSS fields.
			if (type == "text/rcss" ||
				 type == "text/css")
			{
				parser->GetDocumentHeader()->rcss_external.push_back(href);
			}

			// If its an template, add to the template fields
			else if (type == "text/template")
			{
				parser->GetDocumentHeader()->template_resources.push_back(href);
			}

			else
			{
				Log::ParseError(parser->GetSourceURL().GetURL(), parser->GetLineNumber(), "Invalid link type '%s'", type.CString());
			}
		}
		else
		{
			Log::ParseError(parser->GetSourceURL().GetURL(), parser->GetLineNumber(), "Link tag requires type and href attributes");
		}
	}

	// Process script tags
	else if (name == "script")
	{
		// Check if its an external string
		String src = attributes.Get<String>("src", "");
		if (src.Length() > 0)
		{
			parser->GetDocumentHeader()->scripts_external.push_back(src);
		}
	}

	// No elements constructed
	return NULL;
}

bool XMLNodeHandlerHead::ElementEnd(XMLParser* parser, const String& name)
{	
	// When the head tag closes, inject the header into the active document
	if (name == "head")
	{
		Element* element = parser->GetParseFrame()->element;
		if (!element)
			return true;

		ElementDocument* document = element->GetOwnerDocument();
		if (document)
			document->ProcessHeader(parser->GetDocumentHeader());
	}
	return true;
}

bool XMLNodeHandlerHead::ElementData(XMLParser* parser, const String& data)
{
	const String& tag = parser->GetParseFrame()->tag;

	// Store the title
	if (tag == "title")
	{
		SystemInterface* system_interface = GetSystemInterface();
		if (system_interface != NULL)
			system_interface->TranslateString(parser->GetDocumentHeader()->title, data);
	}

	// Store an inline script
	if (tag == "script" && data.Length() > 0)
		parser->GetDocumentHeader()->scripts_inline.push_back(data);

	// Store an inline style
	if (tag == "style" && data.Length() > 0)
		parser->GetDocumentHeader()->rcss_inline.push_back(data);

	return true;
}

void XMLNodeHandlerHead::Release()
{
	delete this;
}

}
}
