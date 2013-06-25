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

#ifndef ROCKETCORESTREAMFILE_H
#define ROCKETCORESTREAMFILE_H

#include <Rocket/Core/Stream.h>
#include <Rocket/Core/Types.h>

namespace Rocket {
namespace Core {

/**
	@author Peter Curry
 */

class StreamFile : public Stream
{
public:
	StreamFile();
	virtual ~StreamFile();

	/// Attempts to open the stream pointing at a given location.
	bool Open(const String& path);
	/// Closes the stream.
	virtual void Close();

	/// Returns the size of this stream (in bytes).
	virtual size_t Length() const;

	/// Returns the position of the stream pointer (in bytes).
	virtual size_t Tell() const;
	/// Sets the stream position (in bytes).
	virtual bool Seek(long offset, int origin) const;

	/// Read from the stream.
	virtual size_t Read(void* buffer, size_t bytes) const;
	using Stream::Read;

	/// Write to the stream at the current position.
	virtual size_t Write(const void* buffer, size_t bytes);
	using Stream::Write;

	/// Truncate the stream to the specified length.
	virtual size_t Truncate(size_t bytes);

	/// Returns true if the stream is ready for reading, false otherwise.
	virtual bool IsReadReady();
	/// Returns false.
	virtual bool IsWriteReady();

private:
	// Determines the length of the stream.
	void GetLength();

	FileHandle file_handle;
	size_t length;
};

}
}

#endif
