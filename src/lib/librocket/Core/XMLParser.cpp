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
#include <Rocket/Core/XMLParser.h>
#include "DocumentHeader.h"
#include <Rocket/Core/Log.h>
#include <Rocket/Core/XMLNodeHandler.h>

namespace Rocket {
namespace Core {

typedef std::map< String, XMLNodeHandler* > NodeHandlers;
static NodeHandlers node_handlers;
static XMLNodeHandler* default_node_handler = NULL;

XMLParser::XMLParser(Element* root)
{
	RegisterCDATATag("script");

	// Add the first frame.
	ParseFrame frame;
	frame.node_handler = NULL;
	frame.child_handler = NULL;
	frame.element = root;
	frame.tag = "";
	stack.push(frame);

	active_handler = NULL;

	header = new DocumentHeader();
}

XMLParser::~XMLParser()
{
	delete header;
}

// Registers a custom node handler to be used to a given tag.
XMLNodeHandler* XMLParser::RegisterNodeHandler(const String& _tag, XMLNodeHandler* handler)
{
	String tag = _tag.ToLower();

	// Check for a default node registration.
	if (tag.Empty())
	{
		if (default_node_handler != NULL)
			default_node_handler->RemoveReference();

		default_node_handler = handler;
		default_node_handler->AddReference();
		return default_node_handler;
	}

	NodeHandlers::iterator i = node_handlers.find(tag);
	if (i != node_handlers.end())
		(*i).second->RemoveReference();

	node_handlers[tag] = handler;
	handler->AddReference();

	return handler;
}

// Releases all registered node handlers. This is called internally.
void XMLParser::ReleaseHandlers()
{
	if (default_node_handler != NULL)
	{
		default_node_handler->RemoveReference();
		default_node_handler = NULL;
	}

	for (NodeHandlers::iterator i = node_handlers.begin(); i != node_handlers.end(); ++i)
		(*i).second->RemoveReference();

	node_handlers.clear();
}

DocumentHeader* XMLParser::GetDocumentHeader()
{
	return header;
}

const URL& XMLParser::GetSourceURL() const
{
	return xml_source->GetSourceURL();
}

// Pushes the default element handler onto the parse stack.
void XMLParser::PushDefaultHandler()
{
	active_handler = default_node_handler;
}

bool XMLParser::PushHandler(const String& tag)
{
	NodeHandlers::iterator i = node_handlers.find(tag.ToLower());
	if (i == node_handlers.end())
		return false;

	active_handler = (*i).second;
	return true;
}

/// Access the current parse frame
const XMLParser::ParseFrame* XMLParser::GetParseFrame() const
{
	return &stack.top();
}

/// Called when the parser finds the beginning of an element tag.
void XMLParser::HandleElementStart(const String& _name, const XMLAttributes& _attributes)
{
	String name = _name.ToLower();
	XMLAttributes attributes;
	
	String key;
	Variant* value;
	int pos = 0;
	while (_attributes.Iterate(pos, key, value))
	{
		attributes.Set(key.ToLower(), *value);
	}

	// Check for a specific handler that will override the child handler.
	NodeHandlers::iterator itr = node_handlers.find(name);
	if (itr != node_handlers.end())
		active_handler = (*itr).second;

	// Store the current active handler, so we can use it through this function (as active handler may change)
	XMLNodeHandler* node_handler = active_handler;

	Element* element = NULL;

	// Get the handler to handle the open tag
	if (node_handler)
	{
		element = node_handler->ElementStart(this, name, attributes);
	}

	// Push onto the stack
	ParseFrame frame;
	frame.node_handler = node_handler;
	frame.child_handler = active_handler;
	frame.element = element != NULL ? element : stack.top().element;
	frame.tag = name;
	stack.push(frame);
}

/// Called when the parser finds the end of an element tag.
void XMLParser::HandleElementEnd(const String& _name)
{
	String name = _name.ToLower();

	// Copy the top of the stack
	ParseFrame frame = stack.top();
	// Pop the frame
	stack.pop();
	// Restore active handler to the previous frame's child handler
	active_handler = stack.top().child_handler;	

	// Check frame names
	if (name != frame.tag)
	{
		Log::Message(Log::LT_ERROR, "Closing tag '%s' mismatched on %s:%d was expecting '%s'.", name.CString(), GetSourceURL().GetURL().CString(), GetLineNumber(), frame.tag.CString());
	}

	// Call element end handler
	if (frame.node_handler)
	{
		frame.node_handler->ElementEnd(this, name);
	}	
}

/// Called when the parser encounters data.
void XMLParser::HandleData(const String& data)
{
	if (stack.top().node_handler)
		stack.top().node_handler->ElementData(this, data);
}

}
}
