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

#ifndef ROCKETCORESTREAMMEMORY_H
#define ROCKETCORESTREAMMEMORY_H

#include <Rocket/Core/Header.h>
#include <Rocket/Core/Stream.h>

namespace Rocket {
namespace Core {

/**
	Memory Byte Stream Class 
	@author Lloyd Weehuizen
 */

class ROCKETCORE_API StreamMemory : public Stream
{
public:
	/// Empty memory stream with default size buffer
	StreamMemory();
	/// Empty memory stream with specified buffer size
	StreamMemory(size_t initial_size);
	/// Read only memory stream based on the existing buffer
	StreamMemory(const byte* buffer, size_t buffer_size);
	/// Copy a memory stream
	StreamMemory(const StreamMemory& copy); 
	virtual ~StreamMemory();

	StreamMemory& operator=(const StreamMemory& copy);

	/// Close the stream
	virtual void Close();	

	/// Are we at the end of the stream
	virtual bool IsEOS() const;

	/// Size of this stream ( in bytes )
	virtual size_t Length() const;

	/// Get Stream position ( in bytes )
	size_t Tell() const;

	/// Set Stream position ( in bytes )
	bool Seek(long offset, int origin) const;

	/// Read from the stream
	using Stream::Read;
	virtual size_t Read(void* buffer, size_t bytes) const;

	/// Peek into the stream
	virtual size_t Peek(void *buffer, size_t bytes) const; 

	/// Write to the stream
	using Stream::Write;
	virtual size_t Write(const void* buffer, size_t bytes);

	/// Truncate the stream to the specified length
	virtual size_t Truncate(size_t bytes);

	/// Push onto the front of the stream
	virtual size_t PushFront(const void* buffer, size_t bytes);

	/// Pop from the front of the stream
	virtual size_t PopFront(size_t bytes);

	/// Raw access to the stream
	const byte* RawStream() const;

	/// Erase a section of the stream
	void Erase(size_t offset, size_t bytes);

	/// Does the stream have data available for reading
	virtual bool IsReadReady();

	/// Is the stream able to accept data now
	virtual bool IsWriteReady();

	/// Sets this streams source URL, useful data that is stored
	/// in memory streams that originated from files
	void SetSourceURL(const URL& url);

private:

	byte* buffer;
	mutable byte* buffer_ptr;
	size_t buffer_size;
	size_t buffer_used;
	bool owns_buffer;
	
	bool Reallocate(size_t size);
};

}
}

#endif
