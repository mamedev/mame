// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

    ui/sliders.cpp

    Internal MAME menus for the user interface.

*********************************************************************/

#include "emu.h"

#include "osdepend.h"

#include "ui/ui.h"
#include "ui/sliders.h"
#include "ui/slider.h"

namespace ui {

menu_sliders::menu_sliders(mame_ui_manager &mui, render_container *container, bool menuless_mode) : menu(mui, container)
{
	m_menuless_mode = m_hidden = menuless_mode;
}

menu_sliders::~menu_sliders()
{
}

//-------------------------------------------------
//	menu_sliders - handle the sliders menu
//-------------------------------------------------

void menu_sliders::handle()
{
	const event *menu_event;

	// process the menu
	menu_event = process(PROCESS_LR_REPEAT | (m_hidden ? PROCESS_CUSTOM_ONLY : 0));
	if (menu_event != nullptr)
	{
		// handle keys if there is a valid item selected
		if (menu_event->itemref != nullptr && menu_event->type == menu_item_type::SLIDER)
		{
			const slider_state *slider = (const slider_state *)menu_event->itemref;
			INT32 curvalue = slider->update(machine(), slider->arg, slider->id, nullptr, SLIDER_NOCHANGE);
			INT32 increment = 0;
			bool alt_pressed = machine().input().code_pressed(KEYCODE_LALT) || machine().input().code_pressed(KEYCODE_RALT);
			bool ctrl_pressed = machine().input().code_pressed(KEYCODE_LCONTROL) || machine().input().code_pressed(KEYCODE_RCONTROL);
			bool shift_pressed = machine().input().code_pressed(KEYCODE_LSHIFT) || machine().input().code_pressed(KEYCODE_RSHIFT);

			switch (menu_event->iptkey)
			{
				// toggle visibility
				case IPT_UI_ON_SCREEN_DISPLAY:
					if (m_menuless_mode)
						menu::stack_pop(machine());
					else
						m_hidden = !m_hidden;
					break;

				// decrease value
				case IPT_UI_LEFT:
					if (alt_pressed && shift_pressed)
						increment = -1;
					if (alt_pressed)
						increment = -(curvalue - slider->minval);
					else if (shift_pressed)
						increment = (slider->incval > 10) ? -(slider->incval / 10) : -1;
					else if (ctrl_pressed)
						increment = -slider->incval * 10;
					else
						increment = -slider->incval;
					break;

				// increase value
				case IPT_UI_RIGHT:
					if (alt_pressed && shift_pressed)
						increment = 1;
					if (alt_pressed)
						increment = slider->maxval - curvalue;
					else if (shift_pressed)
						increment = (slider->incval > 10) ? (slider->incval / 10) : 1;
					else if (ctrl_pressed)
						increment = slider->incval * 10;
					else
						increment = slider->incval;
					break;

				// restore default
				case IPT_UI_SELECT:
					increment = slider->defval - curvalue;
					break;
			}

			// handle any changes
			if (increment != 0)
			{
				INT32 newvalue = curvalue + increment;

				// clamp within bounds
				if (newvalue < slider->minval)
					newvalue = slider->minval;
				if (newvalue > slider->maxval)
					newvalue = slider->maxval;

				// update the slider and recompute the menu
				slider->update(machine(), slider->arg, slider->id, nullptr, newvalue);
				reset(reset_options::REMEMBER_REF);
			}
		}

		// if we are selecting an invalid item and we are hidden, skip to the next one
		else if (m_hidden)
		{
			// if we got here via up or page up, select the previous item
			if (menu_event->iptkey == IPT_UI_UP || menu_event->iptkey == IPT_UI_PAGE_UP)
			{
				selected = (selected + item.size() - 1) % item.size();
				validate_selection(-1);
			}

			// otherwise select the next item
			else if (menu_event->iptkey == IPT_UI_DOWN || menu_event->iptkey == IPT_UI_PAGE_DOWN)
			{
				selected = (selected + 1) % item.size();
				validate_selection(1);
			}
		}
	}
}


//-------------------------------------------------
//	menu_sliders_populate - populate the sliders
//	menu
//-------------------------------------------------

void menu_sliders::populate()
{
	std::string tempstring;

	// add UI sliders
	std::vector<menu_item> ui_sliders = ui().get_slider_list();
	for (menu_item item : ui_sliders)
	{
		if (item.type == menu_item_type::SLIDER)
		{
			slider_state* slider = reinterpret_cast<slider_state *>(item.ref);
			INT32 curval = slider->update(machine(), slider->arg, slider->id, &tempstring, SLIDER_NOCHANGE);
			UINT32 flags = 0;
			if (curval > slider->minval)
				flags |= FLAG_LEFT_ARROW;
			if (curval < slider->maxval)
				flags |= FLAG_RIGHT_ARROW;
			item_append(slider->description, tempstring.c_str(), flags, (void *)slider, menu_item_type::SLIDER);
		}
		else
		{
			item_append(item);
		}
	}

	item_append(menu_item_type::SEPARATOR);

	// add OSD options
	std::vector<menu_item> osd_sliders = machine().osd().get_slider_list();
	for (menu_item item : osd_sliders)
	{
		if (item.type == menu_item_type::SLIDER)
		{
			slider_state* slider = reinterpret_cast<slider_state *>(item.ref);
			INT32 curval = slider->update(machine(), slider->arg, slider->id, &tempstring, SLIDER_NOCHANGE);
			UINT32 flags = 0;
			if (curval > slider->minval)
				flags |= FLAG_LEFT_ARROW;
			if (curval < slider->maxval)
				flags |= FLAG_RIGHT_ARROW;
			item_append(slider->description, tempstring.c_str(), flags, (void *)slider, menu_item_type::SLIDER);
		}
		else
		{
			item_append(item);
		}
	}

	custombottom = 2.0f * ui().get_line_height() + 2.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//	menu_sliders_custom_render - perform our special
//	rendering
//-------------------------------------------------

void menu_sliders::custom_render(void *selectedref, float top, float bottom, float x1, float y1, float x2, float y2)
{
	const slider_state *curslider = (const slider_state *)selectedref;
	if (curslider != nullptr)
	{
		float bar_left, bar_area_top, bar_width, bar_area_height, bar_top, bar_bottom, default_x, current_x;
		float line_height = ui().get_line_height();
		float percentage, default_percentage;
		std::string tempstring;
		float text_height;
		INT32 curval;

		// determine the current value and text
		curval = curslider->update(machine(), curslider->arg, curslider->id, &tempstring, SLIDER_NOCHANGE);

		// compute the current and default percentages
		percentage = (float)(curval - curslider->minval) / (float)(curslider->maxval - curslider->minval);
		default_percentage = (float)(curslider->defval - curslider->minval) / (float)(curslider->maxval - curslider->minval);

		// assemble the text
		tempstring.insert(0, " ").insert(0, curslider->description);

		// move us to the bottom of the screen, and expand to full width
		y2 = 1.0f - UI_BOX_TB_BORDER;
		y1 = y2 - bottom;
		x1 = UI_BOX_LR_BORDER;
		x2 = 1.0f - UI_BOX_LR_BORDER;

		// draw extra menu area
		ui().draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);
		y1 += UI_BOX_TB_BORDER;

		// determine the text height
		ui().draw_text_full(container, tempstring.c_str(), 0, 0, x2 - x1 - 2.0f * UI_BOX_LR_BORDER,
					JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NONE, rgb_t::white, rgb_t::black, nullptr, &text_height);

		// draw the thermometer
		bar_left = x1 + UI_BOX_LR_BORDER;
		bar_area_top = y1;
		bar_width = x2 - x1 - 2.0f * UI_BOX_LR_BORDER;
		bar_area_height = line_height;

		// compute positions
		bar_top = bar_area_top + 0.125f * bar_area_height;
		bar_bottom = bar_area_top + 0.875f * bar_area_height;
		default_x = bar_left + bar_width * default_percentage;
		current_x = bar_left + bar_width * percentage;

		// fill in the percentage
		container->add_rect(bar_left, bar_top, current_x, bar_bottom, UI_SLIDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		// draw the top and bottom lines
		container->add_line(bar_left, bar_top, bar_left + bar_width, bar_top, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container->add_line(bar_left, bar_bottom, bar_left + bar_width, bar_bottom, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		// draw default marker
		container->add_line(default_x, bar_area_top, default_x, bar_top, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container->add_line(default_x, bar_bottom, default_x, bar_area_top + bar_area_height, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		// draw the actual text
		ui().draw_text_full(container, tempstring.c_str(), x1 + UI_BOX_LR_BORDER, y1 + line_height, x2 - x1 - 2.0f * UI_BOX_LR_BORDER,
					JUSTIFY_CENTER, WRAP_WORD, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, &text_height);
	}
}


//-------------------------------------------------
//	slider_ui_handler - pushes the slider
//	menu on the stack and hands off to the
//	standard menu handler
//-------------------------------------------------

UINT32 menu_sliders::ui_handler(render_container *container, mame_ui_manager &mui)
{
	UINT32 result;

	// if this is the first call, push the sliders menu
	if (topmost_menu<menu_sliders>() == nullptr)
		menu::stack_push<menu_sliders>(mui, container, true);

	// handle standard menus
	result = menu::ui_handler(container, mui);

	// if we are cancelled, pop the sliders menu
	if (result == UI_HANDLER_CANCEL)
		menu::stack_pop(mui.machine());

	menu_sliders *uim = topmost_menu<menu_sliders>();
	return uim && uim->m_menuless_mode ? 0 : UI_HANDLER_CANCEL;
}

} // namespace ui
