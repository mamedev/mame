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

#ifndef ROCKETCORELOG_H
#define ROCKETCORELOG_H

#include <Rocket/Core/Header.h>
#include <Rocket/Core/Types.h>

namespace Rocket {
namespace Core {

/**
	libRocket logging API.

	@author Lloyd Weehuizen
 */

class ROCKETCORE_API Log
{
public:
	enum Type
	{
		LT_ALWAYS = 0,
		LT_ERROR,
		LT_ASSERT,
		LT_WARNING,
		LT_INFO,
		LT_DEBUG,
		LT_MAX
	};
	
public:
	/// Initialises the logging interface.
	/// @return True if the logging interface was successful, false if not.
	static bool Initialise();
	/// Shutdown the log interface.
	static void Shutdown();

	/// Log the specified message via the registered log interface
	/// @param[in] type Type of message.
	/// @param[in] format The message, with sprintf-style parameters.
	static void Message(Type type, const char* format, ...);

	/// Log a parse error on the specified file and line number.
	/// @param[in] filename Name of the file with the parse error.
	/// @param[in] line_number Line the error occured on.
	/// @param[in] format The error message, with sprintf-style parameters.
	static void ParseError(const String& filename, int line_number, const char* format, ...);
};

}
}

#endif
