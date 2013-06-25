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
#include "ElementTextDefault.h"
#include "ElementDefinition.h"
#include "ElementStyle.h"
#include "FontFaceHandle.h"
#include <Rocket/Core/ElementDocument.h>
#include <Rocket/Core/ElementUtilities.h>
#include <Rocket/Core/Event.h>
#include <Rocket/Core/FontDatabase.h>
#include <Rocket/Core/Property.h>
#include <Rocket/Core/StyleSheetKeywords.h>

namespace Rocket {
namespace Core {

static bool BuildToken(WString& token, const word*& token_begin, const word* string_end, bool first_token, bool collapse_white_space, bool break_at_endline, int text_transformation);
static bool LastToken(const word* token_begin, const word* string_end, bool collapse_white_space, bool break_at_endline);

ElementTextDefault::ElementTextDefault(const String& tag) : ElementText(tag), colour(255, 255, 255), decoration(this)
{
	dirty_layout_on_change = true;

	generated_decoration = TEXT_DECORATION_NONE;
	decoration_property = TEXT_DECORATION_NONE;

	geometry_dirty = true;

	font_configuration = -1;
	font_dirty = true;
}

ElementTextDefault::~ElementTextDefault()
{
}

void ElementTextDefault::SetText(const WString& _text)
{
	if (text != _text)
	{
		text = _text;

		if (dirty_layout_on_change)
			DirtyLayout();
	}
}

const WString& ElementTextDefault::GetText() const
{
	return text;
}

void ElementTextDefault::OnRender()
{
	FontFaceHandle* font_face_handle = GetFontFaceHandle();
	if (!font_face_handle)
		return;
	
	
	// If our font configuration has potentially changed, update it and force a geometry
	// generation if necessary.
	if (font_dirty &&
		UpdateFontConfiguration())
	{
		geometry_dirty = true;
	}

	// Regenerate the geometry if the colour or font configuration has altered.
	if (geometry_dirty)
		GenerateGeometry(font_face_handle);

	Vector2f translation = GetAbsoluteOffset();
	
	bool render = true;
	Vector2i clip_origin;
	Vector2i clip_dimensions;
	if (GetContext()->GetActiveClipRegion(clip_origin, clip_dimensions))
	{
		float clip_top = clip_origin.y;
		float clip_left = clip_origin.x;
		float clip_right = (clip_origin.x + clip_dimensions.x);
		float clip_bottom = (clip_origin.y + clip_dimensions.y);
		float line_height = GetFontFaceHandle()->GetLineHeight();
		
		render = false;
		for (size_t i = 0; i < lines.size(); ++i)
		{			
			const Line& line = lines[i];
			float x = translation.x + line.position.x;
			float y = translation.y + line.position.y;
			
			bool render_line = !(x > clip_right);
			render_line = render_line && !(x + line.width < clip_left);
			
			render_line = render_line && !(y - line_height > clip_bottom);
			render_line = render_line && !(y < clip_top);
			
			if (render_line)
			{
				render = true;
				break;
			}
		}
	}
	
	if (render)
	{
		for (size_t i = 0; i < geometry.size(); ++i)
			geometry[i].Render(translation);
	}

	if (decoration_property != TEXT_DECORATION_NONE)
		decoration.Render(translation);
}

// Generates a token of text from this element, returning only the width.
bool ElementTextDefault::GenerateToken(float& token_width, int line_begin)
{
	// Bail if we don't have a valid font face.
	FontFaceHandle* font_face_handle = GetFontFaceHandle();
	if (font_face_handle == NULL ||
		line_begin >= (int) text.Length())
		return 0;

	// Determine how we are processing white-space while formatting the text.
	int white_space_property = GetProperty< int >(WHITE_SPACE);
	bool collapse_white_space = white_space_property == WHITE_SPACE_NORMAL ||
								white_space_property == WHITE_SPACE_NOWRAP ||
								white_space_property == WHITE_SPACE_PRE_LINE;
	bool break_at_endline = white_space_property == WHITE_SPACE_PRE ||
							white_space_property == WHITE_SPACE_PRE_WRAP ||
							white_space_property == WHITE_SPACE_PRE_LINE;

	const word* token_begin = text.CString() + line_begin;
	WString token;

	BuildToken(token, token_begin, text.CString() + text.Length(), true, collapse_white_space, break_at_endline, GetProperty< int >(TEXT_TRANSFORM));
	token_width = (float) font_face_handle->GetStringWidth(token, 0);

	return LastToken(token_begin, text.CString() + text.Length(), collapse_white_space, break_at_endline);
}

// Generates a line of text rendered from this element
bool ElementTextDefault::GenerateLine(WString& line, int& line_length, float& line_width, int line_begin, float maximum_line_width, float right_spacing_width, bool trim_whitespace_prefix)
{
	FontFaceHandle* font_face_handle = GetFontFaceHandle();

	// Initialise the output variables.
	line.Clear();
	line_length = 0;
	line_width = 0;

	// Bail if we don't have a valid font face.
	if (font_face_handle == NULL)
		return true;

	// Determine how we are processing white-space while formatting the text.
	int white_space_property = GetProperty< int >(WHITE_SPACE);
	bool collapse_white_space = white_space_property == WHITE_SPACE_NORMAL ||
								white_space_property == WHITE_SPACE_NOWRAP ||
								white_space_property == WHITE_SPACE_PRE_LINE;
	bool break_at_line = maximum_line_width >= 0 &&
						 (white_space_property == WHITE_SPACE_NORMAL ||
						  white_space_property == WHITE_SPACE_PRE_WRAP ||
						  white_space_property == WHITE_SPACE_PRE_LINE);
	bool break_at_endline = white_space_property == WHITE_SPACE_PRE ||
							white_space_property == WHITE_SPACE_PRE_WRAP ||
							white_space_property == WHITE_SPACE_PRE_LINE;

	// Determine what (if any) text transformation we are putting the characters through.
	int text_transform_property = GetProperty< int >(TEXT_TRANSFORM);

	// Starting at the line_begin character, we generate sections of the text (we'll call them tokens) depending on the
	// white-space parsing parameters. Each section is then appended to the line if it can fit. If not, or if an
	// endline is found (and we're processing them), then the line is ended. kthxbai!

	const word* token_begin = text.CString() + line_begin;
	const word* string_end = text.CString() + text.Length();
	while (token_begin != string_end)
	{
		WString token;
		const word* next_token_begin = token_begin;

		// Generate the next token and determine its pixel-length.
		bool break_line = BuildToken(token, next_token_begin, string_end, line.Empty() && trim_whitespace_prefix, collapse_white_space, break_at_endline, text_transform_property);
		int token_width = font_face_handle->GetStringWidth(token, line.Empty() ? 0 : line[line.Length() - 1]);

		// If we're breaking to fit a line box, check if the token can fit on the line before we add it.
		if (break_at_line)
		{
			if ((!line.Empty()) &&
				(((line_width + token_width) > maximum_line_width) ||
				 (LastToken(next_token_begin, string_end, collapse_white_space, break_at_endline) && ((line_width + token_width) > (maximum_line_width - right_spacing_width)))))
			{
				return false;
			}
		}

		// The token can fit on the end of the line, so add it onto the end and increment our width and length
		// counters.
		line += token;
		line_length += (next_token_begin - token_begin);
		line_width += token_width;

		// Break out of the loop if an endline was forced.
		if (break_line)
			return false;

		// Set the beginning of the next token.
		token_begin = next_token_begin;
	}

	return true;
}

// Clears all lines of generated text and prepares the element for generating new lines.
void ElementTextDefault::ClearLines()
{
	// Clear the rendering information.
	for (size_t i = 0; i < geometry.size(); ++i)
		geometry[i].Release(true);

	lines.clear();
	decoration.Release(true);
}

// Adds a new line into the text element.
void ElementTextDefault::AddLine(const Vector2f& line_position, const WString& line)
{
	FontFaceHandle* font_face_handle = GetFontFaceHandle();

	if (font_face_handle == NULL)
		return;

	if (font_dirty)
		UpdateFontConfiguration();

	Vector2f baseline_position = line_position + Vector2f(0.0f, (float) font_face_handle->GetLineHeight() - font_face_handle->GetBaseline());
	lines.push_back(Line(line, baseline_position));

	GenerateGeometry(font_face_handle, lines.back());
	geometry_dirty = false;

	if (decoration_property != TEXT_DECORATION_NONE)
		GenerateDecoration(font_face_handle, lines.back());
}

// Prevents the element from dirtying its document's layout when its text is changed.
void ElementTextDefault::SuppressAutoLayout()
{
	dirty_layout_on_change = false;
}

void ElementTextDefault::OnPropertyChange(const PropertyNameList& changed_properties)
{
	Element::OnPropertyChange(changed_properties);

	bool colour_changed = false;
	bool font_face_changed = false;

	if (changed_properties.find(COLOR) != changed_properties.end())
	{
		// Fetch our (potentially) new colour.
		Colourb new_colour = GetProperty(COLOR)->value.Get< Colourb >();
		colour_changed = colour != new_colour;
		if (colour_changed)
			colour = new_colour;
	}

	if (changed_properties.find(FONT_FAMILY) != changed_properties.end() ||
		changed_properties.find(FONT_CHARSET) != changed_properties.end() ||
		changed_properties.find(FONT_WEIGHT) != changed_properties.end() ||
		changed_properties.find(FONT_STYLE) != changed_properties.end() ||
		changed_properties.find(FONT_SIZE) != changed_properties.end())
	{
		font_face_changed = true;

		geometry.clear();
		font_dirty = true;
	}

	if (changed_properties.find(TEXT_DECORATION) != changed_properties.end())
	{
		decoration_property = GetProperty< int >(TEXT_DECORATION);
		if (decoration_property != TEXT_DECORATION_NONE)
		{
			if (decoration_property != generated_decoration)
			{
				decoration.Release(true);

				FontFaceHandle* font_face_handle = GetFontFaceHandle();
				if (font_face_handle != NULL)
				{
					for (size_t i = 0; i < lines.size(); ++i)
						GenerateDecoration(font_face_handle, lines[i]);
				}

				generated_decoration = decoration_property;
			}
		}
	}

	if (font_face_changed)
	{
		// We have to let our document know we need to be regenerated.
		if (dirty_layout_on_change)
			DirtyLayout();
	}
	else if (colour_changed)
	{
		// Force the geometry to be regenerated.
		geometry_dirty = true;

		// Re-colour the decoration geometry.
		std::vector< Vertex >& vertices = decoration.GetVertices();
		for (size_t i = 0; i < vertices.size(); ++i)
			vertices[i].colour = colour;

		decoration.Release();
	}
}

// Returns the RML of this element
void ElementTextDefault::GetRML(String& content)
{
	text.ToUTF8(content, true);
}

// Forces a reevaluation of applicable font effects.
void ElementTextDefault::DirtyFont()
{
	font_dirty = true;
}

// Updates the configuration this element uses for its font.
bool ElementTextDefault::UpdateFontConfiguration()
{
	if (GetFontFaceHandle() == NULL)
		return false;

	font_dirty = false;

	// Build up a list of all applicable font effects set by our parent nodes.
	FontEffectMap font_effects;
	Element* element = GetParentNode();
	while (element != NULL)
	{
		const ElementDefinition* element_definition = element->GetDefinition();
		if (element_definition != NULL)
			element_definition->GetFontEffects(font_effects, element->GetStyle()->GetActivePseudoClasses());

		element = element->GetParentNode();
	}

	// Request a font layer configuration to match this set of effects. If this is different from
	// our old configuration, then return true to indicate we'll need to regenerate geometry.
	int new_configuration = GetFontFaceHandle()->GenerateLayerConfiguration(font_effects);
	if (new_configuration != font_configuration)
	{
		font_configuration = new_configuration;
		return true;
	}

	return false;
}

// Clears and regenerates all of the text's geometry.
void ElementTextDefault::GenerateGeometry(FontFaceHandle* font_face_handle)
{
	// Release the old geometry ...
	for (size_t i = 0; i < geometry.size(); ++i)
		geometry[i].Release(true);

	/// ... and generate it all again!
	for (size_t i = 0; i < lines.size(); ++i)
		GenerateGeometry(font_face_handle, lines[i]);

	geometry_dirty = false;
}

void ElementTextDefault::GenerateGeometry(FontFaceHandle* font_face_handle, Line& line)
{
	line.width = font_face_handle->GenerateString(geometry, line.text, line.position, colour, font_configuration);
	for (size_t i = 0; i < geometry.size(); ++i)
		geometry[i].SetHostElement(this);
}

// Generates any geometry necessary for rendering a line decoration (underline, strike-through, etc).
void ElementTextDefault::GenerateDecoration(FontFaceHandle* font_face_handle, const Line& line)
{
	Font::Line line_height;
	if (decoration_property == TEXT_DECORATION_OVERLINE)
		line_height = Font::OVERLINE;
	else if (decoration_property == TEXT_DECORATION_LINE_THROUGH)
		line_height = Font::STRIKE_THROUGH;
	else
		line_height = Font::UNDERLINE;

	font_face_handle->GenerateLine(&decoration, line.position, line.width, line_height, colour);
}

static bool BuildToken(WString& token, const word*& token_begin, const word* string_end, bool first_token, bool collapse_white_space, bool break_at_endline, int text_transformation)
{
	ROCKET_ASSERT(token_begin != string_end);

	// Check what the first character of the token is; all we need to know is if it is white-space or not.
	bool parsing_white_space = StringUtilities::IsWhitespace(*token_begin);

	// Loop through the string from the token's beginning until we find an end to the token. This can occur in various
	// places, depending on the white-space processing;
	//  - at the end of a section of non-white-space characters,
	//  - at the end of a section of white-space characters, if we're not collapsing white-space,
	//  - at an endline token, if we're breaking on endlines.
	while (token_begin != string_end)
	{
		bool force_non_whitespace = false;
		word character = *token_begin;

		const word* escape_begin = token_begin;

		// Check for an ampersand; if we find one, we've got an HTML escaped character.
		if (character == '&')
		{
			// Find the terminating ';'.
			while (token_begin != string_end &&
				   *token_begin != ';')
				++token_begin;

			// If we couldn't find the ';', print the token like normal text.
			if (token_begin == string_end)
			{
				token_begin = escape_begin;
			}
			// We could find a ';', parse the escape code. If the escape code is recognised, set the parsed character
			// to the appropriate one. If it is a non-breaking space, prevent it being picked up as whitespace. If it
			// is not recognised, print the token like normal text.
			else
			{
				WString ucs2_escape_code(escape_begin + 1, token_begin);
				String escape_code;
				ucs2_escape_code.ToUTF8(escape_code);

				if (ucs2_escape_code == "lt")
					character = '<';
				else if (ucs2_escape_code == "gt")
					character = '>';
				else if (ucs2_escape_code == "amp")
					character = '&';
				else if (ucs2_escape_code == "nbsp")
				{
					character = ' ';
					force_non_whitespace = true;
				}
				else
					token_begin = escape_begin;
			}
		}

		// Check for an endline token; if we're breaking on endlines and we find one, then return true to indicate a
		// forced break.
		if (break_at_endline &&
			character == '\n')
		{
			token += '\n';
			token_begin++;
			return true;
		}

		// If we've transitioned from white-space characters to non-white-space characters, or vice-versa, then check
		// if should terminate the token; if we're not collapsing white-space, then yes (as sections of white-space are
		// non-breaking), otherwise only if we've transitioned from characters to white-space.
		bool white_space = !force_non_whitespace && StringUtilities::IsWhitespace(character);
		if (white_space != parsing_white_space)
		{
			if (!collapse_white_space)
			{
				// Restore pointer to the beginning of the escaped token, if we processed an escape code.
				token_begin = escape_begin;
				return false;
			}

			// We're collapsing white-space; we only tokenise words, not white-space, so we're only done tokenising
			// once we've begun parsing non-white-space and then found white-space.
			if (!parsing_white_space)
			{
				// However, if we are the last non-whitespace character in the string, and there are trailing
				// whitespace characters after this token, then we need to append a single space to the end of this
				// token.
				if (token_begin != string_end &&
					LastToken(token_begin, string_end, collapse_white_space, break_at_endline))
					token += ' ';

				return false;
			}

			// We've transitioned from white-space to non-white-space, so we append a single white-space character.
			if (!first_token)
				token += ' ';

			parsing_white_space = false;
		}

		// If the current character is white-space, we'll append a space character to the token if we're not collapsing
		// sections of white-space.
		if (white_space)
		{
			if (!collapse_white_space)
				token += ' ';
		}
		else
		{
			if (text_transformation == TEXT_TRANSFORM_UPPERCASE)
			{
				if (character >= 'a' && character <= 'z')
					character += (Rocket::Core::word)('A' - 'a');
			}
			else if (text_transformation == TEXT_TRANSFORM_LOWERCASE)
			{
				if (character >= 'A' && character <= 'Z')
					character -= (Rocket::Core::word)('A' - 'a');
			}

			token += character;
		}

		++token_begin;
	}

	return false;
}

static bool LastToken(const word* token_begin, const word* string_end, bool collapse_white_space, bool break_at_endline)
{
	bool last_token = (token_begin == string_end);
	if (collapse_white_space &&
		!last_token)
	{
		last_token = true;
		const word* character = token_begin;

		while (character != string_end)
		{
			if (!StringUtilities::IsWhitespace(*character) ||
				(break_at_endline && *character == '\n'))
			{
				last_token = false;
				break;
			}

			character++;
		}
	}

	return last_token;
}

}
}
