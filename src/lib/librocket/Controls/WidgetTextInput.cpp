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

#include "WidgetTextInput.h"
#include "ElementTextSelection.h"
#include <Rocket/Core.h>
#include <Rocket/Controls/ElementFormControl.h>
#include <Rocket/Controls/Clipboard.h>
#include <Rocket/Core/SystemInterface.h>

namespace Rocket {
namespace Controls {

const float CURSOR_BLINK_TIME = 0.7f;

WidgetTextInput::WidgetTextInput(ElementFormControl* _parent) : internal_dimensions(0, 0), scroll_offset(0, 0), selection_geometry(_parent), cursor_position(0, 0), cursor_size(0, 0),  cursor_geometry(_parent)
{
	keyboard_showed = false;
	
	parent = _parent;
	parent->SetProperty("white-space", "pre");
	parent->SetProperty("overflow", "hidden");
	parent->SetProperty("drag", "drag");
	parent->SetClientArea(Rocket::Core::Box::CONTENT);

	parent->AddEventListener("resize", this, true);
	parent->AddEventListener("keydown", this, true);
	parent->AddEventListener("textinput", this, true);
	parent->AddEventListener("focus", this, true);
	parent->AddEventListener("blur", this, true);
	parent->AddEventListener("mousedown", this, true);
	parent->AddEventListener("drag", this, true);

	text_element = dynamic_cast< Core::ElementText* >(Core::Factory::InstanceElement(parent, "#text", "#text", Rocket::Core::XMLAttributes()));
	selected_text_element = dynamic_cast< Core::ElementText* >(Core::Factory::InstanceElement(parent, "#text", "#text", Rocket::Core::XMLAttributes()));
	if (text_element != NULL)
	{
		text_element->SuppressAutoLayout();
		parent->AppendChild(text_element, false);
		text_element->RemoveReference();

		selected_text_element->SuppressAutoLayout();
		parent->AppendChild(selected_text_element, false);
		selected_text_element->RemoveReference();
	}

	// Create the dummy selection element.
	selection_element = Core::Factory::InstanceElement(parent, "#selection", "selection", Rocket::Core::XMLAttributes());
	ElementTextSelection* text_selection_element = dynamic_cast< ElementTextSelection* >(selection_element);
	if (text_selection_element != NULL)
	{
		text_selection_element->SetWidget(this);
		parent->AppendChild(text_selection_element, false);
		text_selection_element->RemoveReference();
	}

	edit_index = 0;
	absolute_cursor_index = 0;
	cursor_line_index = 0;
	cursor_character_index = 0;

	ideal_cursor_position = 0;

	max_length = -1;

	selection_anchor_index = 0;
	selection_begin_index = 0;
	selection_length = 0;

	ShowCursor(false);
}

WidgetTextInput::~WidgetTextInput()
{
	parent->RemoveEventListener("resize", this, true);
	parent->RemoveEventListener("keydown", this, true);
	parent->RemoveEventListener("textinput", this, true);
	parent->RemoveEventListener("focus", this, true);
	parent->RemoveEventListener("blur", this, true);
	parent->RemoveEventListener("mousedown", this, true);
	parent->RemoveEventListener("drag", this, true);

	// Remove all the children added by the text widget.
	parent->RemoveChild(text_element);
	parent->RemoveChild(selected_text_element);
	parent->RemoveChild(selection_element);
}

// Sets the value of the text field.
void WidgetTextInput::SetValue(const Core::String& value)
{
	text_element->SetText(value);
	FormatElement();

	UpdateRelativeCursor();
}

// Sets the maximum length (in characters) of this text field.
void WidgetTextInput::SetMaxLength(int _max_length)
{
	if (max_length != _max_length)
	{
		max_length = _max_length;
		if (max_length >= 0)
		{
			const Core::WString& value = GetElement()->GetAttribute< Rocket::Core::String >("value", "");
			if ((int) value.Length() > max_length)
			{
				Rocket::Core::String new_value;
				Core::WString(value.CString(), value.CString() + max_length).ToUTF8(new_value);

				GetElement()->SetAttribute("value", new_value);
			}
		}
	}
}

// Returns the maximum length (in characters) of this text field.
int WidgetTextInput::GetMaxLength() const
{
	return max_length;
}

// Update the colours of the selected text.
void WidgetTextInput::UpdateSelectionColours()
{
	// Determine what the colour of the selected text is. If our 'selection' element has the 'color'
	// attribute set, then use that. Otherwise, use the inverse of our own text colour.
	Rocket::Core::Colourb colour;
	const Rocket::Core::Property* colour_property = selection_element->GetLocalProperty("color");
	if (colour_property != NULL)
		colour = colour_property->Get< Rocket::Core::Colourb >();
	else
	{
		colour = parent->GetProperty< Rocket::Core::Colourb >("color");
		colour.red = 255 - colour.red;
		colour.green = 255 - colour.green;
		colour.blue = 255 - colour.blue;
	}

	// Set the computed text colour on the element holding the selected text.
	selected_text_element->SetProperty("color", Rocket::Core::Property(colour, Rocket::Core::Property::COLOUR));

	// If the 'background-color' property has been set on the 'selection' element, use that as the
	// background colour for the selected text. Otherwise, use the inverse of the selected text
	// colour.
	colour_property = selection_element->GetLocalProperty("background-color");
	if (colour_property != NULL)
		selection_colour = colour_property->Get< Rocket::Core::Colourb >();
	else
		selection_colour = Rocket::Core::Colourb(255 - colour.red, 255 - colour.green, 255 - colour.blue, colour.alpha);
}

// Updates the cursor, if necessary.
void WidgetTextInput::OnUpdate()
{
	if (cursor_timer > 0)
	{
		float current_time = Core::GetSystemInterface()->GetElapsedTime();
		cursor_timer -= (current_time - last_update_time);
		last_update_time = current_time;

		while (cursor_timer <= 0)
		{
			cursor_timer += CURSOR_BLINK_TIME;
			cursor_visible = !cursor_visible;
		}
	}
}

// Renders the cursor, if it is visible.
void WidgetTextInput::OnRender()
{
	Core::ElementUtilities::SetClippingRegion(text_element);

	Rocket::Core::Vector2f text_translation = parent->GetAbsoluteOffset() - Rocket::Core::Vector2f(parent->GetScrollLeft(), parent->GetScrollTop());
	selection_geometry.Render(text_translation);

	if (cursor_visible &&
		!parent->IsDisabled())
	{
		cursor_geometry.Render(text_translation + cursor_position);
	}
}

// Formats the widget's internal content.
void WidgetTextInput::OnLayout()
{
	FormatElement();
	parent->SetScrollLeft(scroll_offset.x);
	parent->SetScrollTop(scroll_offset.y);
}

// Returns the input element's underlying text element.
Core::ElementText* WidgetTextInput::GetTextElement()
{
	return text_element;
}

// Returns the input element's maximum allowed text dimensions.
const Rocket::Core::Vector2f& WidgetTextInput::GetTextDimensions() const
{
	return internal_dimensions;
}

// Gets the parent element containing the widget.
Core::Element* WidgetTextInput::GetElement()
{
	return parent;
}

// Dispatches a change event to the widget's element.
void WidgetTextInput::DispatchChangeEvent()
{
	Rocket::Core::Dictionary parameters;
	parameters.Set("value", GetElement()->GetAttribute< Rocket::Core::String >("value", ""));
	GetElement()->DispatchEvent("change", parameters);
}

// Processes the "keydown" and "textinput" event to write to the input field, and the "focus" and "blur" to set
// the state of the cursor.
void WidgetTextInput::ProcessEvent(Core::Event& event)
{
	if (event == "resize")
	{
		GenerateCursor();

		Rocket::Core::Vector2f text_position = parent->GetBox().GetPosition(Core::Box::CONTENT);
		text_element->SetOffset(text_position, parent);
		selected_text_element->SetOffset(text_position, parent);

		Rocket::Core::Vector2f new_internal_dimensions = parent->GetBox().GetSize(Core::Box::CONTENT);
		if (new_internal_dimensions != internal_dimensions)
		{
			internal_dimensions = new_internal_dimensions;

			FormatElement();
			UpdateCursorPosition();
		}
	}
	else if (parent->IsDisabled())
	{
		return;
	}
	else if (event == "keydown")
	{
		Core::Input::KeyIdentifier key_identifier = (Core::Input::KeyIdentifier) event.GetParameter< int >("key_identifier", 0);
		bool numlock = event.GetParameter< int >("num_lock_key", 0) > 0;
		bool shift = event.GetParameter< int >("shift_key", 0) > 0;
        bool ctrl = event.GetParameter< int >("ctrl_key", 0) > 0;

		switch (key_identifier)
		{
			case Core::Input::KI_NUMPAD4:	if (numlock) break;
			case Core::Input::KI_LEFT:		MoveCursorHorizontal(-1, shift); break;

			case Core::Input::KI_NUMPAD6:	if (numlock) break;
			case Core::Input::KI_RIGHT:		MoveCursorHorizontal(1, shift); break;

			case Core::Input::KI_NUMPAD8:	if (numlock) break;
			case Core::Input::KI_UP:		MoveCursorVertical(-1, shift); break;

			case Core::Input::KI_NUMPAD2:	if (numlock) break;
			case Core::Input::KI_DOWN:		MoveCursorVertical(1, shift); break;

			case Core::Input::KI_NUMPAD7:	if (numlock) break;
			case Core::Input::KI_HOME:		MoveCursorHorizontal(-cursor_character_index, shift); break;

			case Core::Input::KI_NUMPAD1:	if (numlock) break;
			case Core::Input::KI_END:		MoveCursorHorizontal(lines[cursor_line_index].content_length - cursor_character_index, shift); break;

			case Core::Input::KI_BACK:
			{
				if (DeleteCharacter(true))
				{
					FormatElement();
					UpdateRelativeCursor();
				}

				ShowCursor(true);
			}
			break;

			case Core::Input::KI_DECIMAL:	if (numlock) break;
			case Core::Input::KI_DELETE:
			{
				if (DeleteCharacter(false))
				{
					FormatElement();
					UpdateRelativeCursor();
				}

				ShowCursor(true);
			}
			break;

			case Core::Input::KI_NUMPADENTER:
			case Core::Input::KI_RETURN:
			{
				LineBreak();
			}
			break;

			case Core::Input::KI_C:
			{
                if (ctrl)
                    CopySelection();
			}
			break;

			case Core::Input::KI_X:
			{
                if (ctrl)
                {
                    CopySelection();
                    DeleteSelection();
                }
			}
			break;

			case Core::Input::KI_V:
			{
                if (ctrl)
                {
    				const Core::WString clipboard_content = Clipboard::Get();
    				for (size_t i = 0; i < clipboard_content.Length(); ++i)
    				{
    					if (max_length > 0 &&
    						(int) Core::WString(GetElement()->GetAttribute< Rocket::Core::String >("value", "")).Length() < max_length)
    						break;

    					AddCharacter(clipboard_content[i]);
    				}
                }
			}
			break;

			// Ignore tabs so input fields can be navigated through with keys.
			case Core::Input::KI_TAB:
				return;

			default:
			{
			}
			break;
		}

		event.StopPropagation();
	}
	else if (event == "textinput")
	{
		// Only process the text if no modifier keys are pressed.
		if (event.GetParameter< int >("ctrl_key", 0) == 0 &&
			event.GetParameter< int >("alt_key", 0) == 0 &&
			event.GetParameter< int >("meta_key", 0) == 0)
		{
			Rocket::Core::word character = event.GetParameter< Rocket::Core::word >("data", 0);
			if (max_length < 0 || (int) Core::String(GetElement()->GetAttribute< Rocket::Core::String >("value", "")).Length() < max_length)
				AddCharacter(character);
		}

		ShowCursor(true);
		event.StopPropagation();
	}
	else if (event == "focus" &&
			 event.GetTargetElement() == parent)
	{
		UpdateSelection(false);
		ShowCursor(true, false);
	}
	else if (event == "blur" &&
		     event.GetTargetElement() == parent)
	{
		ClearSelection();
		ShowCursor(false, false);
	}	
	else if ((event == "mousedown" ||
			  event == "drag") &&
			 event.GetTargetElement() == parent)
	{
		Rocket::Core::Vector2f mouse_position = Rocket::Core::Vector2f((float) event.GetParameter< int >("mouse_x", 0),
																 (float) event.GetParameter< int >("mouse_y", 0));
		mouse_position -= text_element->GetAbsoluteOffset();

		cursor_line_index = CalculateLineIndex(mouse_position.y);
		cursor_character_index = CalculateCharacterIndex(cursor_line_index, mouse_position.x);

		UpdateAbsoluteCursor();
		UpdateCursorPosition();
		ideal_cursor_position = cursor_position.x;

		UpdateSelection(event == "drag" || event.GetParameter< int >("shift_key", 0) > 0);

		ShowCursor(true);
	}
}

// Adds a new character to the string at the cursor position.
bool WidgetTextInput::AddCharacter(Rocket::Core::word character)
{
	if (!IsCharacterValid(character))
		return false;

	if (selection_length > 0)
		DeleteSelection();

	Core::WString value = GetElement()->GetAttribute< Rocket::Core::String >("value", "");
	value.Insert(GetCursorIndex(), Core::WString(1, character), 1);

	edit_index += 1;

	Rocket::Core::String utf8_value;
	value.ToUTF8(utf8_value);
	GetElement()->SetAttribute("value", utf8_value);
	DispatchChangeEvent();

	UpdateSelection(false);

	return true;
}

// Deletes a character from the string.
bool WidgetTextInput::DeleteCharacter(bool back)
{
	// First, check if we have anything selected; if so, delete that first before we start delete
	// individual characters.
	if (selection_length > 0)
	{
		DeleteSelection();
		DispatchChangeEvent();

		UpdateSelection(false);

		return true;
	}

	Core::WString value = GetElement()->GetAttribute< Rocket::Core::String >("value", "");

	if (back)
	{
		if (GetCursorIndex() == 0)
			return false;

		value.Erase(GetCursorIndex() - 1, 1);
		edit_index -= 1;
	}
	else
	{
		if (GetCursorIndex() == (int) value.Length())
			return false;

		value.Erase(GetCursorIndex(), 1);
	}

	Rocket::Core::String utf8_value;
	value.ToUTF8(utf8_value);
	GetElement()->SetAttribute("value", utf8_value);
	DispatchChangeEvent();

	UpdateSelection(false);

	return true;
}

// Copies the selection (if any) to the clipboard.
void WidgetTextInput::CopySelection()
{
	const Core::String& value = GetElement()->GetAttribute< Rocket::Core::String >("value", "");
	Clipboard::Set(Core::String(value.Substring(selection_begin_index, selection_length)));
}

// Returns the absolute index of the cursor.
int WidgetTextInput::GetCursorIndex() const
{
	return edit_index;
}

// Moves the cursor along the current line.
void WidgetTextInput::MoveCursorHorizontal(int distance, bool select)
{
	absolute_cursor_index += distance;
	absolute_cursor_index = Rocket::Core::Math::Max(0, absolute_cursor_index);

	UpdateRelativeCursor();
	ideal_cursor_position = cursor_position.x;

	UpdateSelection(select);

	ShowCursor(true);
}

// Moves the cursor up and down the text field.
void WidgetTextInput::MoveCursorVertical(int distance, bool select)
{
	bool update_ideal_cursor_position = false;
	cursor_line_index += distance;

	if (cursor_line_index < 0)
	{
		cursor_line_index = 0;
		cursor_character_index = 0;

		update_ideal_cursor_position = true;
	}
	else if (cursor_line_index >= (int) lines.size())
	{
		cursor_line_index = (int) lines.size() - 1;
		cursor_character_index = (int) lines[cursor_line_index].content_length;

		update_ideal_cursor_position = true;
	}
	else
		cursor_character_index = CalculateCharacterIndex(cursor_line_index, ideal_cursor_position);

	UpdateAbsoluteCursor();
	UpdateCursorPosition();

	if (update_ideal_cursor_position)
		ideal_cursor_position = cursor_position.x;

	UpdateSelection(select);

	ShowCursor(true);
}

// Updates the absolute cursor index from the relative cursor indices.
void WidgetTextInput::UpdateAbsoluteCursor()
{
	ROCKET_ASSERT(cursor_line_index < (int) lines.size())

	absolute_cursor_index = cursor_character_index;
	edit_index = cursor_character_index;

	for (int i = 0; i < cursor_line_index; i++)
	{
		absolute_cursor_index += lines[i].content.Length();
		edit_index += lines[i].content.Length() + lines[i].extra_characters;
	}
}

// Updates the relative cursor indices from the absolute cursor index.
void WidgetTextInput::UpdateRelativeCursor()
{
	int num_characters = 0;
	edit_index = absolute_cursor_index;

	for (size_t i = 0; i < lines.size(); i++)
	{
		if (num_characters + lines[i].content_length >= absolute_cursor_index)
		{
			cursor_line_index = (int) i;
			cursor_character_index = absolute_cursor_index - num_characters;

			UpdateCursorPosition();

			return;
		}

		num_characters += (int) lines[i].content.Length();
		edit_index += lines[i].extra_characters;
	}

	// We shouldn't ever get here; this means we actually couldn't find where the absolute cursor said it was. So we'll
	// just set the relative cursors to the very end of the text field, and update the absolute cursor to point here.
	cursor_line_index = (int) lines.size() - 1;
	cursor_character_index = lines[cursor_line_index].content_length;
	absolute_cursor_index = num_characters;
	edit_index = num_characters;

	UpdateCursorPosition();
}

// Calculates the line index under a specific vertical position.
int WidgetTextInput::CalculateLineIndex(float position)
{
	float line_height = (float) Core::ElementUtilities::GetLineHeight(parent);
	int line_index = Rocket::Core::Math::RealToInteger(position / line_height);
	return Rocket::Core::Math::Clamp(line_index, 0, (int) (lines.size() - 1));
}

// Calculates the character index along a line under a specific horizontal position.
int WidgetTextInput::CalculateCharacterIndex(int line_index, float position)
{
	int character_index = 0;
	float line_width = 0;

	while (character_index < lines[line_index].content_length)
	{
		float next_line_width = (float) Core::ElementUtilities::GetStringWidth(text_element, lines[line_index].content.Substring(0, character_index));
		if (next_line_width > position)
		{
			if (position - line_width < next_line_width - position)
				return Rocket::Core::Math::Max(0, character_index - 1);
			else
				return character_index;
		}

		line_width = next_line_width;
		character_index++;
	}

	return character_index;
}

// Shows or hides the cursor.
void WidgetTextInput::ShowCursor(bool show, bool move_to_cursor)
{
	if (show)
	{
		cursor_visible = true;
		SetKeyboardActive(true);
		keyboard_showed = true;
		
		cursor_timer = CURSOR_BLINK_TIME;
		last_update_time = Core::GetSystemInterface()->GetElapsedTime();

		// Shift the cursor into view.
		if (move_to_cursor)
		{
			float minimum_scroll_top = (cursor_position.y + cursor_size.y) - parent->GetClientHeight();
			if (parent->GetScrollTop() < minimum_scroll_top)
				parent->SetScrollTop(minimum_scroll_top);
			else if (parent->GetScrollTop() > cursor_position.y)
				parent->SetScrollTop(cursor_position.y);

			float minimum_scroll_left = (cursor_position.x + cursor_size.x) - parent->GetClientWidth();
			if (parent->GetScrollLeft() < minimum_scroll_left)
				parent->SetScrollLeft(minimum_scroll_left);
			else if (parent->GetScrollLeft() > cursor_position.x)
				parent->SetScrollLeft(cursor_position.x);

			scroll_offset.x = parent->GetScrollLeft();
			scroll_offset.y = parent->GetScrollTop();
		}
	}
	else
	{
		cursor_visible = false;
		cursor_timer = -1;
		last_update_time = 0;
		if (keyboard_showed)
		{
			SetKeyboardActive(false);
			keyboard_showed = false;
		}
	}
}

// Formats the element, laying out the text and inserting scrollbars as appropriate.
void WidgetTextInput::FormatElement()
{
	Core::ElementScroll* scroll = parent->GetElementScroll();
	float width = parent->GetBox().GetSize(Core::Box::PADDING).x;

	int x_overflow_property = parent->GetProperty< int >("overflow-x");
	int y_overflow_property = parent->GetProperty< int >("overflow-y");

	if (x_overflow_property == Core::OVERFLOW_SCROLL)
		scroll->EnableScrollbar(Core::ElementScroll::HORIZONTAL, width);
	else
		scroll->DisableScrollbar(Core::ElementScroll::HORIZONTAL);

	if (y_overflow_property == Core::OVERFLOW_SCROLL)
		scroll->EnableScrollbar(Core::ElementScroll::VERTICAL, width);
	else
		scroll->DisableScrollbar(Core::ElementScroll::VERTICAL);

	// Format the text and determine its total area.
	Rocket::Core::Vector2f content_area = FormatText();

	// If we're set to automatically generate horizontal scrollbars, check for that now.
	if (x_overflow_property == Core::OVERFLOW_AUTO)
	{
		if (parent->GetClientWidth() < content_area.x)
			scroll->EnableScrollbar(Core::ElementScroll::HORIZONTAL, width);
	}

	// Now check for vertical overflow. If we do turn on the scrollbar, this will cause a reflow.
	if (y_overflow_property == Core::OVERFLOW_AUTO)
	{
		if (parent->GetClientHeight() < content_area.y)
		{
			scroll->EnableScrollbar(Core::ElementScroll::VERTICAL, width);
			content_area = FormatText();

			if (x_overflow_property == Core::OVERFLOW_AUTO &&
				parent->GetClientWidth() < content_area.y)
			{
				scroll->EnableScrollbar(Core::ElementScroll::HORIZONTAL, width);
			}
		}
	}

	parent->SetContentBox(Rocket::Core::Vector2f(0, 0), content_area);
	scroll->FormatScrollbars();
}

// Formats the input element's text field.
Rocket::Core::Vector2f WidgetTextInput::FormatText()
{
	absolute_cursor_index = edit_index;

	Rocket::Core::Vector2f content_area(0, 0);

	// Clear the old lines, and all the lines in the text elements.
	lines.clear();
	text_element->ClearLines();
	selected_text_element->ClearLines();

	// Clear the selection background geometry, and get the vertices and indices so the new geo can
	// be generated.
	selection_geometry.Release(true);
	std::vector< Core::Vertex >& selection_vertices = selection_geometry.GetVertices();
	std::vector< int >& selection_indices = selection_geometry.GetIndices();

	// Determine the line-height of the text element.
	int line_height = Rocket::Core::ElementUtilities::GetLineHeight(parent);

	int line_begin = 0;
	Rocket::Core::Vector2f line_position(0, 0);
	bool last_line = false;

	// Keep generating lines until all the text content is placed.
	do
	{
		Line line;
		line.extra_characters = 0;
		float line_width;

		// Generate the next line.
		last_line = text_element->GenerateLine(line.content, line.content_length, line_width, line_begin, parent->GetClientWidth() - cursor_size.x, 0, false);

		// If this line terminates in a soft-return, then the line may be leaving a space or two behind as an orphan.
		// If so, we must append the orphan onto the line even though it will push the line outside of the input
		// field's bounds.
		bool soft_return = false;
		if (!last_line &&
			(line.content.Empty() ||
			 line.content[line.content.Length() - 1] != '\n'))
		{
			soft_return = true;

			const Core::WString& text = text_element->GetText();
			Core::WString orphan;
			for (int i = 1; i >= 0; --i)
			{
				int index = line_begin + line.content_length + i;
				if (index >= (int) text.Length())
					continue;

				if (text[index] != ' ')
				{
					orphan = "";
					continue;
				}

				int next_index = index + 1;
				if (!orphan.Empty() ||
					next_index >= (int) text.Length() ||
					text[next_index] != ' ')
					orphan += ' ';
			}

			if (!orphan.Empty())
			{
				line.content += orphan;
				line.content_length += (int) orphan.Length();
				line_width += Core::ElementUtilities::GetStringWidth(text_element, orphan);
			}
		}


		// Now that we have the string of characters appearing on the new line, we split it into
		// three parts; the unselected text appearing before any selected text on the line, the
		// selected text on the line, and any unselected text after the selection.
		Core::WString pre_selection, selection, post_selection;
		GetLineSelection(pre_selection, selection, post_selection, line.content, line_begin);

		// The pre-selected text is placed, if there is any (if the selection starts on or before
		// the beginning of this line, then this will be empty).
		if (!pre_selection.Empty())
		{
			text_element->AddLine(line_position, pre_selection);
			line_position.x += Core::ElementUtilities::GetStringWidth(text_element, pre_selection);
		}

		// If there is any selected text on this line, place it in the selected text element and
		// generate the geometry for its background.
		if (!selection.Empty())
		{
			selected_text_element->AddLine(line_position, selection);
			int selection_width = Core::ElementUtilities::GetStringWidth(selected_text_element, selection);

			selection_vertices.resize(selection_vertices.size() + 4);
			selection_indices.resize(selection_indices.size() + 6);
			Core::GeometryUtilities::GenerateQuad(&selection_vertices[selection_vertices.size() - 4], &selection_indices[selection_indices.size() - 6], line_position, Rocket::Core::Vector2f((float) selection_width, (float) line_height), selection_colour, selection_vertices.size() - 4);

			line_position.x += selection_width;
		}

		// If there is any unselected text after the selection on this line, place it in the
		// standard text element after the selected text.
		if (!post_selection.Empty())
			text_element->AddLine(line_position, post_selection);


		// Update variables for the next line.
		line_begin += line.content_length;
		line_position.x = 0;
		line_position.y += line_height;

		// Grow the content area width-wise if this line is the longest so far, and push the
		// height out.
		content_area.x = Rocket::Core::Math::Max(content_area.x, line_width + cursor_size.x);
		content_area.y = line_position.y;

		// Push a trailing '\r' token onto the back to indicate a soft return if necessary.
		if (soft_return)
		{
			line.content += '\r';
			line.extra_characters -= 1;

			if (edit_index >= line_begin)
				absolute_cursor_index += 1;
		}

		// Push the new line into our array of lines, but first check if its content length needs to be truncated to
		// dodge a trailing endline.
		if (!line.content.Empty() &&
			line.content[line.content.Length() - 1] == '\n')
			line.content_length -= 1;
		lines.push_back(line);
	}
	while (!last_line);

	return content_area;
}

// Generates the text cursor.
void WidgetTextInput::GenerateCursor()
{
	// Generates the cursor.
	cursor_geometry.Release();

	std::vector< Core::Vertex >& vertices = cursor_geometry.GetVertices();
	vertices.resize(4);

	std::vector< int >& indices = cursor_geometry.GetIndices();
	indices.resize(6);

	cursor_size.x = 1;
	cursor_size.y = (float) Core::ElementUtilities::GetLineHeight(text_element) + 2;
	Core::GeometryUtilities::GenerateQuad(&vertices[0], &indices[0], Rocket::Core::Vector2f(0, 0), cursor_size, parent->GetProperty< Rocket::Core::Colourb >("color"));
}

void WidgetTextInput::UpdateCursorPosition()
{
	if (text_element->GetFontFaceHandle() == NULL)
		return;

	cursor_position.x = (float) Core::ElementUtilities::GetStringWidth(text_element, lines[cursor_line_index].content.Substring(0, cursor_character_index));
	cursor_position.y = -1 + cursor_line_index * (float) Core::ElementUtilities::GetLineHeight(text_element);
}

// Expand the text selection to the position of the cursor.
void WidgetTextInput::UpdateSelection(bool selecting)
{
	if (!selecting)
	{
		selection_anchor_index = edit_index;
		ClearSelection();
	}
	else
	{
		int new_begin_index;
		int new_end_index;

		if (edit_index > selection_anchor_index)
		{
			new_begin_index = selection_anchor_index;
			new_end_index = edit_index;
		}
		else
		{
			new_begin_index = edit_index;
			new_end_index = selection_anchor_index;
		}

		if (new_begin_index != selection_begin_index ||
			new_end_index - new_begin_index != selection_length)
		{
			selection_begin_index = new_begin_index;
			selection_length = new_end_index - new_begin_index;

			FormatText();
		}
	}
}

// Removes the selection of text.
void WidgetTextInput::ClearSelection()
{
	if (selection_length > 0)
	{
		selection_length = 0;
		FormatElement();
	}
}

// Deletes all selected text and removes the selection.
void WidgetTextInput::DeleteSelection()
{
	if (selection_length > 0)
	{
		const Core::WString& value = GetElement()->GetAttribute< Rocket::Core::String >("value", "");

		Rocket::Core::String new_value;
		Core::WString(value.Substring(0, selection_begin_index) + value.Substring(selection_begin_index + selection_length)).ToUTF8(new_value);
		GetElement()->SetAttribute("value", new_value);

		// Move the cursor to the beginning of the old selection.
		absolute_cursor_index = selection_begin_index;
		UpdateRelativeCursor();

		// Erase our record of the selection.
		ClearSelection();
	}
}

// Split one line of text into three parts, based on the current selection.
void WidgetTextInput::GetLineSelection(Core::WString& pre_selection, Core::WString& selection, Core::WString& post_selection, const Core::WString& line, int line_begin)
{
	// Check if we have any selection at all, and if so if the selection is on this line.
	if (selection_length <= 0 ||
		selection_begin_index + selection_length < line_begin ||
		selection_begin_index > line_begin + (int) line.Length())
	{
		pre_selection = line;
		return;
	}

	// Split the line up into its three parts, depending on the size and placement of the selection.
	pre_selection = line.Substring(0, Rocket::Core::Math::Max(0, selection_begin_index - line_begin));
	selection = line.Substring(Rocket::Core::Math::Max(0, selection_begin_index - line_begin), Rocket::Core::Math::Max(0, selection_length + Rocket::Core::Math::Min(0, selection_begin_index - line_begin)));
	post_selection = line.Substring(selection_begin_index + selection_length - line_begin);
}

void WidgetTextInput::SetKeyboardActive(bool active)
{
	Core::SystemInterface* system = Core::GetSystemInterface();
	if (system) {
		if (active) 
		{
			system->ActivateKeyboard();
		} else 
		{
			system->DeactivateKeyboard();
		}
	}
}
	
}
}
