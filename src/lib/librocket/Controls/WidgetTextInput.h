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

#ifndef ROCKETCONTROLSWIDGETTEXTINPUT_H
#define ROCKETCONTROLSWIDGETTEXTINPUT_H

#include <Rocket/Core/EventListener.h>
#include <Rocket/Core/Geometry.h>
#include <Rocket/Core/WString.h>
#include <Rocket/Core/Vertex.h>

namespace Rocket {
namespace Core {

class ElementText;

}

namespace Controls {

class ElementFormControl;

/**
	An abstract widget for editing and navigating around a text field.

	@author Peter Curry
 */

class WidgetTextInput : public Core::EventListener
{
public:
	WidgetTextInput(ElementFormControl* parent);
	virtual ~WidgetTextInput();

	/// Sets the value of the text field.
	/// @param[in] value The new value to set on the text field.
	virtual void SetValue(const Core::String& value);

	/// Sets the maximum length (in characters) of this text field.
	/// @param[in] max_length The new maximum length of the text field. A number lower than zero will mean infinite characters.
	void SetMaxLength(int max_length);
	/// Returns the maximum length (in characters) of this text field.
	/// @return The maximum number of characters allowed in this text field.
	int GetMaxLength() const;

	/// Update the colours of the selected text.
	void UpdateSelectionColours();

	/// Updates the cursor, if necessary.
	void OnUpdate();
	/// Renders the cursor, if it is visible.
	void OnRender();
	/// Formats the widget's internal content.
	void OnLayout();

	/// Returns the input element's underlying text element.
	Core::ElementText* GetTextElement();
	/// Returns the input element's maximum allowed text dimensions.
	const Rocket::Core::Vector2f& GetTextDimensions() const;

protected:
	/// Processes the "keydown" and "textinput" event to write to the input field, and the "focus" and
	/// "blur" to set the state of the cursor.
	virtual void ProcessEvent(Core::Event& event);

	/// Adds a new character to the string at the cursor position.
	/// @param[in] character The character to add to the string.
	/// @return True if the character was successfully added, false otherwise.
	bool AddCharacter(Rocket::Core::word character);
	/// Deletes a character from the string.
	/// @param[in] backward True to delete a character behind the cursor, false for in front of the cursor.
	/// @return True if a character was deleted, false otherwise.
	bool DeleteCharacter(bool back);
	/// Returns true if the given character is permitted in the input field, false if not.
	/// @param[in] character The character to validate.
	/// @return True if the character is allowed, false if not.
	virtual bool IsCharacterValid(Rocket::Core::word character) = 0;
	/// Called when the user pressed enter.
	virtual void LineBreak() = 0;

	/// Returns the absolute index of the cursor.
	int GetCursorIndex() const;

	/// Gets the parent element containing the widget.
	Core::Element* GetElement();

	/// Dispatches a change event to the widget's element.
	void DispatchChangeEvent();

private:
	/// Moves the cursor along the current line.
	/// @param[in] x How far to move the cursor.
	/// @param[in] select True if the movement will also move the selection cursor, false if not.
	void MoveCursorHorizontal(int distance, bool select);
	/// Moves the cursor up and down the text field.
	/// @param[in] x How far to move the cursor.
	/// @param[in] select True if the movement will also move the selection cursor, false if not.
	void MoveCursorVertical(int distance, bool select);

	/// Updates the absolute cursor index from the relative cursor indices.
	void UpdateAbsoluteCursor();
	/// Updates the relative cursor indices from the absolute cursor index.
	void UpdateRelativeCursor();

	/// Calculates the line index under a specific vertical position.
	/// @param[in] position The position to query.
	/// @return The index of the line under the mouse cursor.
	int CalculateLineIndex(float position);
	/// Calculates the character index along a line under a specific horizontal position.
	/// @param[in] line_index The line to query.
	/// @param[in] position The position to query.
	/// @return The index of the character under the mouse cursor.
	int CalculateCharacterIndex(int line_index, float position);

	/// Shows or hides the cursor.
	/// @param[in] show True to show the cursor, false to hide it.
	/// @param[in] move_to_cursor True to force the cursor to be visible, false to not scroll the widget.
	void ShowCursor(bool show, bool move_to_cursor = true);

	/// Formats the element, laying out the text and inserting scrollbars as appropriate.
	void FormatElement();
	/// Formats the input element's text field.
	/// @return The content area of the element.
	Rocket::Core::Vector2f FormatText();

	/// Generates the text cursor.
	void GenerateCursor();
	/// Updates the position to render the cursor.
	void UpdateCursorPosition();

	/// Expand or shrink the text selection to the position of the cursor.
	/// @param[in] selecting True if the new position of the cursor should expand / contract the selection area, false if it should only set the anchor for future selections.
	void UpdateSelection(bool selecting);
	/// Removes the selection of text.
	void ClearSelection();
	/// Deletes all selected text and removes the selection.
	void DeleteSelection();
	/// Copies the selection (if any) to the clipboard.
	void CopySelection();

	/// Split one line of text into three parts, based on the current selection.
	/// @param[out] pre_selection The section of unselected text before any selected text on the line.
	/// @param[out] selection The section of selected text on the line.
	/// @param[out] post_selection The section of unselected text after any selected text on the line. If there is no selection on the line, then this will be empty.
	/// @param[in] line The text making up the line.
	/// @param[in] line_begin The absolute index at the beginning of the line.
	void GetLineSelection(Core::WString& pre_selection, Core::WString& selection, Core::WString& post_selection, const Core::WString& line, int line_begin);

	struct Line
	{
		// The contents of the line (including the trailing endline, if that terminated the line).
		Core::WString content;
		// The length of the editable characters on the line (excluding any trailing endline).
		int content_length;

		// The number of extra characters at the end of the content that are not present in the actual value; in the
		// case of a soft return, this may be negative.
		int extra_characters;
	};

	ElementFormControl* parent;

	Core::ElementText* text_element;
	Core::ElementText* selected_text_element;
	Rocket::Core::Vector2f internal_dimensions;
	Rocket::Core::Vector2f scroll_offset;

	typedef std::vector< Line > LineList;
	LineList lines;

	int max_length;

	int edit_index;

	int absolute_cursor_index;
	int cursor_line_index;
	int cursor_character_index;

	// Selection. The start and end indices of the selection are in absolute coordinates.
	Core::Element* selection_element;
	int selection_anchor_index;
	int selection_begin_index;
	int selection_length;

	// The colour of the background of selected text.
	Rocket::Core::Colourb selection_colour;
	// The selection background.
	Core::Geometry selection_geometry;

	// Cursor visibility and timings.
	float cursor_timer;
	bool cursor_visible;
	bool keyboard_showed;
	/// Activate or deactivate keyboard (for touchscreen devices)
	/// @param[in] active True if need activate keyboard, false if need deactivate.
	void SetKeyboardActive(bool active);

	float last_update_time;

	// The cursor geometry.
	float ideal_cursor_position;
	Rocket::Core::Vector2f cursor_position;
	Rocket::Core::Vector2f cursor_size;
	Core::Geometry cursor_geometry;
};

}
}

#endif
