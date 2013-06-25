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

#ifndef ROCKETCOREFONTFAMILY_H
#define ROCKETCOREFONTFAMILY_H

#include <Rocket/Core/Font.h>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace Rocket {
namespace Core {

class FontFace;
class FontFaceHandle;

/**
	@author Peter Curry
 */

class FontFamily
{
public:
	FontFamily(const String& name);
	~FontFamily();

	/// Adds a new face to the family.
	/// @param[in] ft_face The previously loaded FreeType face.
	/// @param[in] style The style of the new face.
	/// @param[in] weight The weight of the new face.
	/// @param[in] release_stream True if the application must free the face's memory stream.
	/// @return True if the face was loaded successfully, false otherwise.
	bool AddFace(FT_Face ft_face, Font::Style style, Font::Weight weight, bool release_stream);

	/// Returns a handle to the most appropriate font in the family, at the correct size.
	/// @param[in] charset The set of characters in the handle, as a comma-separated list of unicode ranges.
	/// @param[in] style The style of the desired handle.
	/// @param[in] weight The weight of the desired handle.
	/// @param[in] size The size of desired handle, in points.
	/// @return A valid handle if a matching (or closely matching) font face was found, NULL otherwise.
	FontFaceHandle* GetFaceHandle(const String& charset, Font::Style style, Font::Weight weight, int size);

private:
	String name;

	typedef std::vector< FontFace* > FontFaceList;
	FontFaceList font_faces;
};

}
}

#endif
