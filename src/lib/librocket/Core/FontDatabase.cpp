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
#include <Rocket/Core/FontDatabase.h>
#include "FontFamily.h"
#include <Rocket/Core.h>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace Rocket {
namespace Core {

FontDatabase* FontDatabase::instance = NULL;

typedef std::map< String, FontEffect* > FontEffectCache;
FontEffectCache font_effect_cache;

static FT_Library ft_library = NULL;

FontDatabase::FontDatabase()
{
	ROCKET_ASSERT(instance == NULL);
	instance = this;
}

FontDatabase::~FontDatabase()
{
	ROCKET_ASSERT(instance == this);
	instance = NULL;
}

bool FontDatabase::Initialise()
{
	if (instance == NULL)
	{
		new FontDatabase();

		FT_Error result = FT_Init_FreeType(&ft_library);
		if (result != 0)
		{
			Log::Message(Log::LT_ERROR, "Failed to initialise FreeType, error %d.", result);
			Shutdown();
			return false;
		}
	}

	return true;
}

void FontDatabase::Shutdown()
{
	if (instance != NULL)
	{
		for (FontFamilyMap::iterator i = instance->font_families.begin(); i != instance->font_families.end(); ++i)
			delete (*i).second;

		if (ft_library != NULL)
		{
			FT_Done_FreeType(ft_library);
			ft_library = NULL;
		}

		delete instance;
	}
}

// Adds a new font face to the database, ignoring any family, style and weight information stored in the face itself.
bool FontDatabase::LoadFontFace(const String& file_name)
{
	FT_Face ft_face = (FT_Face) instance->LoadFace(file_name);
	if (ft_face == NULL)
	{
		Log::Message(Log::LT_ERROR, "Failed to load font face from %s.", file_name.CString());
		return false;
	}

	Font::Style style = ft_face->style_flags & FT_STYLE_FLAG_ITALIC ? Font::STYLE_ITALIC : Font::STYLE_NORMAL;
	Font::Weight weight = ft_face->style_flags & FT_STYLE_FLAG_BOLD ? Font::WEIGHT_BOLD : Font::WEIGHT_NORMAL;

	if (instance->AddFace(ft_face, ft_face->family_name, style, weight, true))
	{
		Log::Message(Log::LT_INFO, "Loaded font face %s %s (from %s).", ft_face->family_name, ft_face->style_name, file_name.CString());
		return true;
	}
	else
	{
		Log::Message(Log::LT_ERROR, "Failed to load font face %s %s (from %s).", ft_face->family_name, ft_face->style_name, file_name.CString());
		return false;
	}
}

// Loads a new font face.
bool FontDatabase::LoadFontFace(const String& file_name, const String& family, Font::Style style, Font::Weight weight)
{
	FT_Face ft_face = (FT_Face) instance->LoadFace(file_name);
	if (ft_face == NULL)
	{
		Log::Message(Log::LT_ERROR, "Failed to load font face from %s.", file_name.CString());
		return false;
	}

	if (instance->AddFace(ft_face, family, style, weight, true))
	{
		Log::Message(Log::LT_INFO, "Loaded font face %s %s (from %s).", ft_face->family_name, ft_face->style_name, file_name.CString());
		return true;
	}
	else
	{
		Log::Message(Log::LT_ERROR, "Failed to load font face %s %s (from %s).", ft_face->family_name, ft_face->style_name, file_name.CString());
		return false;
	}
}

// Adds a new font face to the database, loading from memory.
bool FontDatabase::LoadFontFace(const byte* data, int data_length, const String& family, Font::Style style, Font::Weight weight)
{
	FT_Face ft_face = (FT_Face) instance->LoadFace(data, data_length, "memory", false);
	if (ft_face == NULL)
	{
		Log::Message(Log::LT_ERROR, "Failed to load font face from byte stream.");
		return false;
	}

	if (instance->AddFace(ft_face, family, style, weight, false))
	{
		Log::Message(Log::LT_INFO, "Loaded font face %s %s (from byte stream).", ft_face->family_name, ft_face->style_name);
		return true;
	}
	else
	{
		Log::Message(Log::LT_ERROR, "Failed to load font face %s %s (from byte stream).", ft_face->family_name, ft_face->style_name);
		return false;
	}
}

// Returns a handle to a font face that can be used to position and render text.
FontFaceHandle* FontDatabase::GetFontFaceHandle(const String& family, const String& charset, Font::Style style, Font::Weight weight, int size)
{
	FontFamilyMap::iterator iterator = instance->font_families.find(family);
	if (iterator == instance->font_families.end())
		return NULL;

	return (*iterator).second->GetFaceHandle(charset, style, weight, size);
}

// Returns a font effect, either a newly-instanced effect from the factory or an identical shared
// effect.
FontEffect* FontDatabase::GetFontEffect(const String& name, const PropertyDictionary& properties)
{
	// The caching here should be moved into the Factory for optimal behaviour. This system has a
	// few shortfalls:
	//  * ignores default properties
	//  * could be shared with decorators as well

	// Generate a key so we can distinguish unique property sets quickly.
	typedef std::list< std::pair< String, String > > PropertyList;
	PropertyList sorted_properties;
	for (PropertyMap::const_iterator property_iterator = properties.GetProperties().begin(); property_iterator != properties.GetProperties().end(); ++property_iterator)
	{
		// Skip the font-effect declaration.
		if (property_iterator->first == "font-effect")
			continue;

		PropertyList::iterator insert = sorted_properties.begin();
		while (insert != sorted_properties.end() &&
			   insert->first < property_iterator->first)
		   ++insert;

		sorted_properties.insert(insert, PropertyList::value_type(property_iterator->first, property_iterator->second.Get< String >()));
	}

	// Generate the font effect's key from the properties.
	String key = name + ";";
	for (PropertyList::iterator i = sorted_properties.begin(); i != sorted_properties.end(); ++i)
		key += i->first + ":" + i->second + ";";

	// Check if we have a previously instanced effect.
	FontEffectCache::iterator i = font_effect_cache.find(key);
	if (i != font_effect_cache.end())
	{
		FontEffect* effect = i->second;
		effect->AddReference();

		return effect;
	}

	FontEffect* font_effect = Factory::InstanceFontEffect(name, properties);
	if (font_effect == NULL)
		return NULL;

	font_effect_cache[key] = font_effect;
	return font_effect;
}

// Removes a font effect from the font database's cache.
void FontDatabase::ReleaseFontEffect(const FontEffect* effect)
{
	for (FontEffectCache::iterator i = font_effect_cache.begin(); i != font_effect_cache.end(); ++i)
	{
		if (i->second == effect)
		{
			font_effect_cache.erase(i);
			return;
		}
	}
}

// Adds a loaded face to the appropriate font family.
bool FontDatabase::AddFace(void* face, const String& family, Font::Style style, Font::Weight weight, bool release_stream)
{
	FontFamily* font_family = NULL;
	FontFamilyMap::iterator iterator = instance->font_families.find(family);
	if (iterator != instance->font_families.end())
		font_family = (*iterator).second;
	else
	{
		font_family = new FontFamily(family);
		instance->font_families[family] = font_family;
	}

	return font_family->AddFace((FT_Face) face, style, weight, release_stream);
}

// Loads a FreeType face.
void* FontDatabase::LoadFace(const String& file_name)
{
	FileInterface* file_interface = GetFileInterface();
	FileHandle handle = file_interface->Open(file_name);

	if (!handle)
	{
		return false;
	}

	size_t length = file_interface->Length(handle);

	FT_Byte* buffer = new FT_Byte[length];
	file_interface->Read(buffer, length, handle);
	file_interface->Close(handle);

	return LoadFace(buffer, length, file_name, true);
}

// Loads a FreeType face from memory.
void* FontDatabase::LoadFace(const byte* data, int data_length, const String& source, bool local_data)
{
	FT_Face face = NULL;
	int error = FT_New_Memory_Face(ft_library, (const FT_Byte*) data, data_length, 0, &face);
	if (error != 0)
	{
		Log::Message(Log::LT_ERROR, "FreeType error %d while loading face from %s.", error, source.CString());
		if (local_data)
			delete[] data;

		return NULL;
	}

	// Initialise the character mapping on the face.
	if (face->charmap == NULL)
	{
		FT_Select_Charmap(face, FT_ENCODING_APPLE_ROMAN);
		if (face->charmap == NULL)
		{
			Log::Message(Log::LT_ERROR, "Font face (from %s) does not contain a Unicode or Apple Roman character map.", source.CString());
			FT_Done_Face(face);
			if (local_data)
				delete[] data;

			return NULL;
		}
	}

	return face;
}

}
}
