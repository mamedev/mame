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

#ifndef ROCKETDEBUGGERDEBUGGER_H
#define ROCKETDEBUGGERDEBUGGER_H

#include <Rocket/Core/Types.h>
#include <Rocket/Debugger/Header.h>

namespace Rocket {
namespace Core {

class Context;

}

namespace Debugger {

/// Initialises the debug plugin. The debugger will be loaded into the given context.
/// @param[in] context The Rocket context to load the debugger into. The debugging tools will be displayed on this context. If this context is destroyed, the debugger will be released.
/// @return True if the debugger was successfully initialised
ROCKETDEBUGGER_API bool Initialise(Core::Context* context);

/// Sets the context to be debugged.
/// @param[in] context The context to be debugged.
/// @return True if the debugger is initialised and the context was switched, false otherwise.
ROCKETDEBUGGER_API bool SetContext(Core::Context* context);

/// Sets the visibility of the debugger.
/// @param[in] visibility True to show the debugger, false to hide it.
ROCKETDEBUGGER_API void SetVisible(bool visibility);
/// Returns the visibility of the debugger.
/// @return True if the debugger is visible, false if not.
ROCKETDEBUGGER_API bool IsVisible();

}
}

#endif
