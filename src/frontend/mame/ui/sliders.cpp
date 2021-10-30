// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

    ui/sliders.cpp

    Internal MAME menus for the user interface.

*********************************************************************/

#include "emu.h"
#include "ui/sliders.h"

#include "ui/slider.h"
#include "ui/ui.h"

#include "osdepend.h"


namespace ui {

menu_sliders::menu_sliders(mame_ui_manager &mui, render_container &container, bool menuless_mode)
	: menu(mui, container)
	, m_menuless_mode(menuless_mode)
	, m_hidden(menuless_mode)
{
	set_one_shot(menuless_mode);
	set_process_flags(PROCESS_LR_REPEAT | (m_hidden ? PROCESS_CUSTOM_ONLY : 0));
}

menu_sliders::~menu_sliders()
{
}

//-------------------------------------------------
//  menu_sliders - handle the sliders menu
//-------------------------------------------------

void menu_sliders::handle(event const *ev)
{
	// process the menu
	if (ev)
	{
		// handle keys if there is a valid item selected
		if (ev->itemref && (ev->type == menu_item_type::SLIDER))
		{
			const slider_state *slider = (const slider_state *)ev->itemref;
			int32_t curvalue = slider->update(nullptr, SLIDER_NOCHANGE);
			int32_t increment = 0;
			bool alt_pressed = machine().input().code_pressed(KEYCODE_LALT) || machine().input().code_pressed(KEYCODE_RALT);
			bool ctrl_pressed = machine().input().code_pressed(KEYCODE_LCONTROL) || machine().input().code_pressed(KEYCODE_RCONTROL);
			bool shift_pressed = machine().input().code_pressed(KEYCODE_LSHIFT) || machine().input().code_pressed(KEYCODE_RSHIFT);

			switch (ev->iptkey)
			{
				// toggle visibility
				case IPT_UI_ON_SCREEN_DISPLAY:
					if (m_menuless_mode)
					{
						stack_pop();
					}
					else
					{
						m_hidden = !m_hidden;
						set_process_flags(PROCESS_LR_REPEAT | (m_hidden ? PROCESS_CUSTOM_ONLY : 0));
					}
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
				int32_t newvalue = curvalue + increment;

				// clamp within bounds
				if (newvalue < slider->minval)
					newvalue = slider->minval;
				if (newvalue > slider->maxval)
					newvalue = slider->maxval;

				// update the slider and recompute the menu
				slider->update(nullptr, newvalue);
				reset(reset_options::REMEMBER_REF);
			}
		}

		// if we are selecting an invalid item and we are hidden, skip to the next one
		else if (m_hidden)
		{
			if (ev->iptkey == IPT_UI_UP || ev->iptkey == IPT_UI_PAGE_UP)
			{
				// if we got here via up or page up, select the previous item
				if (is_first_selected())
					select_last_item();
				else
				{
					set_selected_index(selected_index() - 1);
					validate_selection(-1);
				}
			}
			else if (ev->iptkey == IPT_UI_DOWN || ev->iptkey == IPT_UI_PAGE_DOWN)
			{
				// otherwise select the next item
				if (is_last_selected())
					select_first_item();
				else
				{
					set_selected_index(selected_index() + 1);
					validate_selection(1);
				}
			}
		}
	}
}


//-------------------------------------------------
//  menu_sliders_populate - populate the sliders
//  menu
//-------------------------------------------------

void menu_sliders::populate(float &customtop, float &custombottom)
{
	std::string tempstring;

	// add UI sliders
	std::vector<menu_item> ui_sliders = ui().get_slider_list();
	for (const menu_item &item : ui_sliders)
	{
		if (item.type == menu_item_type::SLIDER)
		{
			slider_state *const slider = reinterpret_cast<slider_state *>(item.ref);
			bool display(true);
#if 0
			// FIXME: this test should be reimplemented in a dedicated menu
			if (slider->id >= SLIDER_ID_ADJUSTER && slider->id <= SLIDER_ID_ADJUSTER_LAST)
				display = reinterpret_cast<ioport_field *>(slider->arg)->enabled();
#endif
			if (display)
			{
				int32_t curval = slider->update(&tempstring, SLIDER_NOCHANGE);
				uint32_t flags = 0;
				if (curval > slider->minval)
					flags |= FLAG_LEFT_ARROW;
				if (curval < slider->maxval)
					flags |= FLAG_RIGHT_ARROW;
				item_append(slider->description, tempstring, flags, (void *)slider, menu_item_type::SLIDER);
			}
		}
		else
		{
			item_append(item);
		}
	}

	item_append(menu_item_type::SEPARATOR);

	// add OSD options
	std::vector<menu_item> osd_sliders = machine().osd().get_slider_list();
	for (const menu_item &item : osd_sliders)
	{
		if (item.type == menu_item_type::SLIDER)
		{
			slider_state* slider = reinterpret_cast<slider_state *>(item.ref);
			int32_t curval = slider->update(&tempstring, SLIDER_NOCHANGE);
			uint32_t flags = 0;
			if (curval > slider->minval)
				flags |= FLAG_LEFT_ARROW;
			if (curval < slider->maxval)
				flags |= FLAG_RIGHT_ARROW;
			item_append(slider->description, tempstring, flags, (void *)slider, menu_item_type::SLIDER);
		}
		else
		{
			item_append(item);
		}
	}

	custombottom = 2.0f * ui().get_line_height() + 2.0f * ui().box_tb_border();
}

//-------------------------------------------------
//  menu_sliders_custom_render - perform our special
//  rendering
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
		int32_t curval;

		// determine the current value and text
		curval = curslider->update(&tempstring, SLIDER_NOCHANGE);

		// compute the current and default percentages
		percentage = (float)(curval - curslider->minval) / (float)(curslider->maxval - curslider->minval);
		default_percentage = (float)(curslider->defval - curslider->minval) / (float)(curslider->maxval - curslider->minval);

		// assemble the text
		tempstring.insert(0, " ").insert(0, curslider->description);

		// move us to the bottom of the screen, and expand to full width
		const float lr_border = ui().box_lr_border() * machine().render().ui_aspect(&container());
		y2 = 1.0f - ui().box_tb_border();
		y1 = y2 - bottom;
		x1 = lr_border;
		x2 = 1.0f - lr_border;

		// draw extra menu area
		ui().draw_outlined_box(container(), x1, y1, x2, y2, ui().colors().background_color());
		y1 += ui().box_tb_border();

		// determine the text height
		ui().draw_text_full(
				container(),
				tempstring,
				0, 0, x2 - x1 - 2.0f * lr_border,
				text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE,
				mame_ui_manager::NONE, rgb_t::white(), rgb_t::black(), nullptr, &text_height);

		// draw the thermometer
		bar_left = x1 + lr_border;
		bar_area_top = y1;
		bar_width = x2 - x1 - 2.0f * lr_border;
		bar_area_height = line_height;

		// compute positions
		bar_top = bar_area_top + 0.125f * bar_area_height;
		bar_bottom = bar_area_top + 0.875f * bar_area_height;
		default_x = bar_left + bar_width * default_percentage;
		current_x = bar_left + bar_width * percentage;

		// fill in the percentage
		container().add_rect(bar_left, bar_top, current_x, bar_bottom, ui().colors().slider_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		// draw the top and bottom lines
		container().add_line(bar_left, bar_top, bar_left + bar_width, bar_top, UI_LINE_WIDTH, ui().colors().border_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container().add_line(bar_left, bar_bottom, bar_left + bar_width, bar_bottom, UI_LINE_WIDTH, ui().colors().border_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		// draw default marker
		container().add_line(default_x, bar_area_top, default_x, bar_top, UI_LINE_WIDTH, ui().colors().border_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container().add_line(default_x, bar_bottom, default_x, bar_area_top + bar_area_height, UI_LINE_WIDTH, ui().colors().border_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		// draw the actual text
		ui().draw_text_full(
				container(),
				tempstring,
				x1 + lr_border, y1 + line_height, x2 - x1 - 2.0f * lr_border,
				text_layout::text_justify::CENTER, text_layout::word_wrapping::WORD,
				mame_ui_manager::NORMAL, ui().colors().text_color(), ui().colors().text_bg_color(), nullptr, &text_height);
	}
}

} // namespace ui
