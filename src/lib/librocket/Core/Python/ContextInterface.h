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

#ifndef ROCKETCOREPYTHONCONTEXTINTERFACE_H
#define ROCKETCOREPYTHONCONTEXTINTERFACE_H

#include "ContextDocumentProxy.h"

namespace Rocket {
namespace Core {
namespace Python {

/**
	Installs and manages the Python interface for libRocket Context objects.

	@author Peter Curry
 */

class ContextInterface
{
public:
	/// Initialises the Python context interface.
	static bool InitialisePythonInterface();
	/// Initialise the Rocket context interface.
	static void InitialiseRocketInterface();

	/// The "AddEventListener" function bound into Python context objects instead of the C++
	/// function.
	/// @param[in] self The calling context.
	/// @param[in] event The event to bind to.
	/// @param[in] script The script fragment to execute.
	/// @param[in] in_capture_phase True if binding on the capture phase, false if the bubble phase.
	static void AddEventListener(Context* self, const char* event, const char* script, bool in_capture_phase);
	/// The "CreateDocument" function bound into Python context objects instead of the C++
	/// function.
	/// @param[in] self The calling context.
	/// @param[in] tag The tag name for the instanced document.
	/// @return The new document.
	static python::object CreateDocument(Context* self, const char* tag);
	/// The "LoadDocument" function bound into Python context objects instead of the C++ function.
	/// @param[in] self The calling context.
	/// @param[in] document_path The path to load the document from.
	/// @return The loaded document.
	static python::object LoadDocument(Context* self, const char* document_path);
	/// The "LoadDocument" function bound into Python context objects instead of the C++ function.
	/// @param[in] self The calling context.
	/// @param[in] stream The document stream
	/// @return The loaded document.
	static python::object LoadDocumentFromMemory(Context* self, const char* stream);
	/// The "LoadMouseCursor" function bound into Python context objects instead of the C++
	/// function.
	/// @param[in] self The calling context.
	/// @param[in] document_path The path to load the cursor from.
	/// @return The loaded cursor.
	static python::object LoadMouseCursor(Context* self, const char* document_path);
	/// Returns the document proxy object for the 'document' property.
	/// @param[in] self The calling context.
	/// @return The proxy object.
	static ContextDocumentProxy GetDocuments(Context* self);
};

}
}
}

#endif
