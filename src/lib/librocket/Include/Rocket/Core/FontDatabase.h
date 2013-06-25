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

#ifndef ROCKETCOREFONTDATABASE_H
#define ROCKETCOREFONTDATABASE_H

#include <Rocket/Core/StringUtilities.h>
#include <Rocket/Core/Header.h>
#include <Rocket/Core/Font.h>

namespace Rocket {
namespace Core {

class FontEffect;
class FontFamily;
class FontFaceHandle;
class PropertyDictionary;

/**
	The font database contains all font families currently in use by Rocket.

	@author Peter Curry
 */

class ROCKETCORE_API FontDatabase
{
public:
	static bool Initialise();
	static void Shutdown();

	/// Adds a new font face to the database. The face's family, style and weight will be determined from the face itself.
	/// @param[in] file_name The file to load the face from.
	/// @return True if the face was loaded successfully, false otherwise.
	static bool LoadFontFace(const String& file_name);
	/// Adds a new font face to the database, ignoring any family, style and weight information stored in the face itself.
	/// @param[in] file_name The file to load the face from.
	/// @param[in] family The family to add the face to.
	/// @param[in] style The style of the face (normal or italic).
	/// @param[in] weight The weight of the face (normal or bold).
	/// @return True if the face was loaded successfully, false otherwise.
	static bool LoadFontFace(const String& file_name, const String& family, Font::Style style, Font::Weight weight);
	/// Adds a new font face to the database, loading from memory.
	/// @param[in] data The font data.
	/// @param[in] data_length Length of the data.
	/// @param[in] family The family to add the face to.
	/// @param[in] style The style of the face (normal or italic).
	/// @param[in] weight The weight of the face (normal or bold).
	/// @return True if the face was loaded successfully, false otherwise.
	static bool LoadFontFace(const byte* data, int data_length, const String& family, Font::Style style, Font::Weight weight);

	/// Returns a handle to a font face that can be used to position and render text. This will return the closest match
	/// it can find, but in the event a font family is requested that does not exist, NULL will be returned instead of a
	/// valid handle.
	/// @param[in] family The family of the desired font handle.
	/// @param[in] charset The set of characters required in the font face, as a comma-separated list of unicode ranges.
	/// @param[in] style The style of the desired font handle.
	/// @param[in] weight The weight of the desired font handle.
	/// @param[in] size The size of desired handle, in points.
	/// @return A valid handle if a matching (or closely matching) font face was found, NULL otherwise.
	static FontFaceHandle* GetFontFaceHandle(const String& family, const String& charset, Font::Style style, Font::Weight weight, int size);

	/// Returns a font effect, either a newly-instanced effect from the factory or an identical
	/// shared effect.
	/// @param[in] name The name of the desired font effect type.
	/// @param[in] properties The properties associated with the font effect.
	/// @return The requested font effect, or NULL if the font effect could not be found or instanced.
	static FontEffect* GetFontEffect(const String& name, const PropertyDictionary& properties);

	/// Removes a font effect from the font database's cache.
	/// @param[in] The effect to release.
	static void ReleaseFontEffect(const FontEffect* effect);

private:
	FontDatabase(void);
	~FontDatabase(void);

	// Adds a loaded face to the appropriate font family.
	bool AddFace(void* face, const String& family, Font::Style style, Font::Weight weight, bool release_stream);
	// Loads a FreeType face.
	void* LoadFace(const String& file_name);
	// Loads a FreeType face from memory.
	void* LoadFace(const byte* data, int data_length, const String& source, bool local_data);

	typedef std::map< String, FontFamily*, StringUtilities::StringComparei > FontFamilyMap;
	FontFamilyMap font_families;

	static FontDatabase* instance;
};

}
}

#endif
