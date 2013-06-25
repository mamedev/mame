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

#include "precompiled.h"
#include "FontFace.h"
#include "FontFaceHandle.h"
#include <Rocket/Core/Log.h>

namespace Rocket {
namespace Core {

FontFace::FontFace(FT_Face _face, Font::Style _style, Font::Weight _weight, bool _release_stream)
{
	face = _face;
	style = _style;
	weight = _weight;

	release_stream = _release_stream;
}

FontFace::~FontFace()
{
	for (HandleMap::iterator iterator = handles.begin(); iterator != handles.end(); ++iterator)
	{
		HandleList& handle_list = (*iterator).second;
		for (size_t i = 0; i < handle_list.size(); ++i)
			handle_list[i]->RemoveReference();
	}

	ReleaseFace();
}

// Returns the style of the font face.
Font::Style FontFace::GetStyle() const
{
	return style;
}

// Returns the weight of the font face.
Font::Weight FontFace::GetWeight() const
{
	return weight;
}

// Returns a handle for positioning and rendering this face at the given size.
FontFaceHandle* FontFace::GetHandle(const String& _raw_charset, int size)
{
	UnicodeRangeList charset;

	HandleMap::iterator iterator = handles.find(size);
	if (iterator != handles.end())
	{
		const HandleList& handles = (*iterator).second;

		// Check all the handles if their charsets match the requested one exactly (ie, were specified by the same
		// string).
		String raw_charset(_raw_charset);
		for (size_t i = 0; i < handles.size(); ++i)
		{
			if (handles[i]->GetRawCharset() == _raw_charset)
			{
				handles[i]->AddReference();
				return handles[i];
			}
		}

		// Check all the handles if their charsets contain the requested charset.
		if (!UnicodeRange::BuildList(charset, raw_charset))
		{
			Log::Message(Log::LT_ERROR, "Invalid font charset '%s'.", _raw_charset.CString());
			return NULL;
		}

		for (size_t i = 0; i < handles.size(); ++i)
		{
			bool range_contained = true;

			const UnicodeRangeList& handle_charset = handles[i]->GetCharset();
			for (size_t j = 0; j < charset.size() && range_contained; ++j)
			{
				if (!charset[j].IsContained(handle_charset))
					range_contained = false;
			}

			if (range_contained)
			{
				handles[i]->AddReference();
				return handles[i];
			}
		}
	}

	// See if this face has been released.
	if (face == NULL)
	{
		Log::Message(Log::LT_WARNING, "Font face has been released, unable to generate new handle.");
		return NULL;
	}

	// Construct and initialise the new handle.
	FontFaceHandle* handle = new FontFaceHandle();
	if (!handle->Initialise(face, _raw_charset, size))
	{
		handle->RemoveReference();
		return NULL;
	}

	// Save the handle, and add a reference for the callee. The initial reference will be removed when the font face
	// releases it.
	if (iterator != handles.end())
		(*iterator).second.push_back(handle);
	else
		handles[size] = HandleList(1, handle);

	handle->AddReference();

	return handle;
}

// Releases the face's FreeType face structure.
void FontFace::ReleaseFace()
{
	if (face != NULL)
	{
		FT_Byte* face_memory = face->stream->base;
		FT_Done_Face(face);

		if (release_stream)
			delete[] face_memory;

		face = NULL;
	}
}

}
}
