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

#ifndef ROCKETCOREPLUGIN_H
#define ROCKETCOREPLUGIN_H

#include <Rocket/Core/Header.h>

namespace Rocket {
namespace Core {

class Element;
class ElementDocument;
class Context;

/**
	Generic Interface for plugins to Rocket.

	@author Lloyd Weehuizen
 */

class ROCKETCORE_API Plugin
{
public:
	virtual ~Plugin();

	enum EventClasses
	{
		EVT_BASIC		= (1 << 0),		// Initialise, Shutdown, ContextCreate, ContextDestroy
		EVT_DOCUMENT	= (1 << 1),		// DocumentOpen, DocumentLoad, DocumentUnload
		EVT_ELEMENT		= (1 << 2),		// ElementCreate, ElementDestroy

		EVT_ALL			= EVT_BASIC | EVT_DOCUMENT | EVT_ELEMENT
	};
	/// Called when the plugin is registered to determine
	/// which of the above event types the plugin is interested in
	virtual int GetEventClasses();

	/// Called when Rocket is initialised, or immediately when the plugin registers itself if 
	/// Rocket has already been initialised.
	virtual void OnInitialise();
	/// Called when Rocket shuts down.
	virtual void OnShutdown();

	/// Called when a new context is created.
	virtual void OnContextCreate(Context* context);
	/// Called when a context is destroyed.
	virtual void OnContextDestroy(Context* context);

	/// Called when a document load request occurs, before the document's file is opened.
	virtual void OnDocumentOpen(Context* context, const String& document_path);
	/// Called when a document is successfully loaded from file or instanced, initialised and added
	/// to its context. This is called before the document's 'load' event.
	virtual void OnDocumentLoad(ElementDocument* document);
	/// Called when a document is unloaded from its context. This is called after the document's
	/// 'unload' event.
	virtual void OnDocumentUnload(ElementDocument* document);

	/// Called when a new element is created.
	virtual void OnElementCreate(Element* element);
	/// Called when an element is destroyed.
	virtual void OnElementDestroy(Element* element);
};

}
}

#endif
