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

#ifndef ROCKETSHELL_H
#define ROCKETSHELL_H

#include <Rocket/Core/Types.h>
#include <Rocket/Core/SystemInterface.h>


/**
	Shell functions for creating windows, attaching OpenGL and handling input in a platform independent way.
	@author Lloyd Weehuizen
 */

class Shell
{
public:
	/// Initialise the shell.
	/// @param[in] path The path (relative to the current working directory) of the application's working directory.
	static bool Initialise(const Rocket::Core::String& path);
	/// Shutdown the shell.
	static void Shutdown();

	/// Loads the default fonts from the given path.
	static void LoadFonts(const char* directory);

	/// Returns the path to the application's executable.
	static const Rocket::Core::String& GetExecutablePath();

	/// Open a platform specific window, optionally initialising an OpenGL context on it.
	/// @param[in] title Title of the window.
	/// @param[in] attach_opengl Attach and opengl context to the window.
	static bool OpenWindow(const char* title, bool attach_opengl);
	/// Close the active window.
	static void CloseWindow();

	/// Returns a platform-dependent handle to the window.
	static void* GetWindowHandle();

	/// Flips the OpenGL buffers.
	static void FlipBuffers();

	/// Run the event loop, calling the idle function every frame.
	typedef void (*ShellIdleFunction)();
	static void EventLoop(ShellIdleFunction idle_function);
	static void RequestExit();

	/// Display an error message.
	static void DisplayError(const char* fmt, ...);
	/// Log a message to the debugger.
	static void Log(const char* fmt, ...);

	/// Get the number of seconds that have passed since shell startup.
	static float GetElapsedTime();	

private:
	static Rocket::Core::String executable_path;
};

#include "ShellRenderInterfaceOpenGL.h"
#include "ShellSystemInterface.h"

#endif
