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

#ifndef ROCKETCOREFILEINTERFACE_H
#define ROCKETCOREFILEINTERFACE_H

#include <Rocket/Core/Header.h>
#include <Rocket/Core/Types.h>
#include <Rocket/Core/ReferenceCountable.h>

namespace Rocket {
namespace Core {

/**
	The abstract base class for application-specific file I/O.

	By default, Rocket will use a file interface implementing the standard C file functions. If this is not sufficient,
	or your application wants more control over file I/O, this class should be derived, instanced, and installed
	through Core::SetFileInterface() before you initialise Rocket.

	@author Peter Curry
 */

class ROCKETCORE_API FileInterface : public ReferenceCountable
{
public:
	FileInterface();
	virtual ~FileInterface();

	/// Opens a file.
	/// @param file The file handle to write to.
	/// @return A valid file handle, or NULL on failure
	virtual FileHandle Open(const String& path) = 0;
	/// Closes a previously opened file.
	/// @param file The file handle previously opened through Open().
	virtual void Close(FileHandle file) = 0;

	/// Reads data from a previously opened file.
	/// @param buffer The buffer to be read into.
	/// @param size The number of bytes to read into the buffer.
	/// @param file The handle of the file.
	/// @return The total number of bytes read into the buffer.
	virtual size_t Read(void* buffer, size_t size, FileHandle file) = 0;
	/// Seeks to a point in a previously opened file.
	/// @param file The handle of the file to seek.
	/// @param offset The number of bytes to seek.
	/// @param origin One of either SEEK_SET (seek from the beginning of the file), SEEK_END (seek from the end of the file) or SEEK_CUR (seek from the current file position).
	/// @return True if the operation completed successfully, false otherwise.
	virtual bool Seek(FileHandle file, long offset, int origin) = 0;
	/// Returns the current position of the file pointer.
	/// @param file The handle of the file to be queried.
	/// @return The number of bytes from the origin of the file.
	virtual size_t Tell(FileHandle file) = 0;

	/// Returns the length of the file.
	/// The default implementation uses Seek & Tell.
	/// @param file The handle of the file to be queried.
	/// @return The length of the file in bytes.
	virtual size_t Length(FileHandle file);

	/// Called when this file interface is released.
	virtual void Release();

protected:
	virtual void OnReferenceDeactivate();
};

}
}

#endif
