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

#ifndef ROCKETCOREXMLPARSER_H
#define ROCKETCOREXMLPARSER_H

#include <stack>
#include <Rocket/Core/Header.h>
#include <Rocket/Core/BaseXMLParser.h>

namespace Rocket {
namespace Core {

class DocumentHeader;
class Element;
class XMLNodeHandler;

/**
	Rocket's XML parsing engine. The factory creates an instance of this class for each RML parse.

	@author Lloyd Weehuizen
 */

class ROCKETCORE_API XMLParser : public BaseXMLParser
{
public:
	XMLParser(Element* root);
	~XMLParser();

	/// Registers a custom node handler to be used to a given tag.
	/// @param[in] tag The tag the custom parser will handle.
	/// @param[in] handler The custom handler.
	/// @return The registered XML node handler.
	static XMLNodeHandler* RegisterNodeHandler(const String& tag, XMLNodeHandler* handler);
	/// Releases all registered node handlers. This is called internally.
	static void ReleaseHandlers();

	/// Returns the XML document's header.
	/// @return The document header.
	DocumentHeader* GetDocumentHeader();
	/// Returns the source URL of this parse.
	/// @return The URL of the parsing stream.
	const URL& GetSourceURL() const;

	// The parse stack.
	struct ParseFrame
	{
		// Tag being parsed.
		String tag;

		// Element representing this frame.
		Element* element;

		// Handler used for this frame.
		XMLNodeHandler* node_handler;

		// The default handler used for this frame's children.
		XMLNodeHandler* child_handler;
	};

	/// Pushes an element handler onto the parse stack for parsing child elements.
	/// @param[in] tag The tag the handler was registered with.
	/// @return True if an appropriate handler was found and pushed onto the stack, false if not.
	bool PushHandler(const String& tag);
	/// Pushes the default element handler onto the parse stack.
	void PushDefaultHandler();

	/// Access the current parse frame.
	/// @return The parser's current parse frame.
	const ParseFrame* GetParseFrame() const;

protected:
	/// Called when the parser finds the beginning of an element tag.
	virtual void HandleElementStart(const String& name, const XMLAttributes& attributes);
	/// Called when the parser finds the end of an element tag.
	virtual void HandleElementEnd(const String& name);
	/// Called when the parser encounters data.
	virtual void HandleData(const String& data);

private:
	// The header of the document being parsed.
	DocumentHeader* header;
	// The root of the entire parse.
	Element* parse_root;

	// The active node handler.
	XMLNodeHandler* active_handler;

	// The parser stack.
	typedef std::stack< ParseFrame > ParserStack;
	ParserStack stack;
};

}
}

#endif
