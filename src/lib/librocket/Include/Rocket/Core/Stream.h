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

#ifndef ROCKETCORESTREAM_H
#define ROCKETCORESTREAM_H

#include <Rocket/Core/Header.h>
#include <Rocket/Core/ReferenceCountable.h>
#include <Rocket/Core/Types.h>
#include <Rocket/Core/URL.h>
#include <list>

namespace Rocket {
namespace Core {

class StreamListener;

/**
	Abstract class for a media-independent byte stream.
	@author Lloyd Weehuizen
 */

class ROCKETCORE_API Stream : public ReferenceCountable
{
	public:
		// Stream modes.
		enum StreamMode
		{
			MODE_WRITE = 1 << 0,
			MODE_APPEND = 1 << 1,
			MODE_READ = 1 << 2,
			MODE_ASYNC = 1 << 3,

			MODE_MASK = MODE_WRITE | MODE_APPEND | MODE_READ
		};

		Stream();
		virtual ~Stream();

		/// Closes the stream.
		virtual void Close();

		/// Returns the mode the stream was opened in.
		int GetStreamMode() const;

		/// Obtain the source url of this stream (if available)
		const URL& GetSourceURL() const;

		/// Are we at the end of the stream
		virtual bool IsEOS() const;

		/// Returns the size of this stream (in bytes).
		virtual size_t Length() const = 0;

		/// Returns the position of the stream pointer (in bytes).
		virtual size_t Tell() const = 0;
		/// Sets the stream position (in bytes).
		virtual bool Seek(long offset, int origin) const = 0;

		/// Read from the stream.
		virtual size_t Read(void* buffer, size_t bytes) const = 0;
		/// Read from the stream into another stream.
		virtual size_t Read(Stream* stream, size_t bytes) const;
		/// Read from the stream and append to the string buffer
		virtual size_t Read(String& buffer, size_t bytes) const;
		/// Read from the stream, without increasing the stream offset.
		virtual size_t Peek(void* buffer, size_t bytes) const;

		/// Write to the stream at the current position.
		virtual size_t Write(const void* buffer, size_t bytes) = 0;
		/// Write to this stream from another stream.
		virtual size_t Write(const Stream* stream, size_t bytes);
		/// Write a character array to the stream.
		virtual size_t Write(const char* string);
		/// Write a string to the stream
		virtual size_t Write(const String& string);

		/// Truncate the stream to the specified length.
		virtual size_t Truncate(size_t bytes) = 0;

		/// Push onto the front of the stream.
		virtual size_t PushFront(const void* buffer, size_t bytes);
		/// Push onto the back of the stream.
		virtual size_t PushBack(const void* buffer, size_t bytes);

		/// Pop from the front of the stream.
		virtual size_t PopFront(size_t bytes);
		/// Pop from the back of the stream.
		virtual size_t PopBack(size_t bytes);

		/// Returns true if the stream is ready for reading, false otherwise.
		/// This is usually only implemented on streams supporting asynchronous
		/// operations.
		virtual bool IsReadReady() = 0;
		/// Returns true if the stream is ready for writing, false otherwise.
		/// This is usually only implemented on streams supporting asynchronous
		/// operations.
		virtual bool IsWriteReady() = 0;

	protected:

		/// Sets the mode on the stream; should be called by a stream when it is opened.
		void SetStreamDetails(const URL& url, int stream_mode);		

		/// Deletes the stream.
		virtual void OnReferenceDeactivate();

	private:
		URL url;
		int stream_mode;
};

}
}

#endif
