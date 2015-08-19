// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

    miscmenu.c

    Internal MAME menus for the user interface.

*********************************************************************/

#include "emu.h"

#include "osdepend.h"
#include "uiinput.h"
#include "ui/ui.h"
#include "ui/sliders.h"


ui_menu_sliders::ui_menu_sliders(running_machine &machine, render_container *container, bool _menuless_mode) : ui_menu(machine, container)
{
	menuless_mode = hidden = _menuless_mode;
}

ui_menu_sliders::~ui_menu_sliders()
{
}

/*-------------------------------------------------
    menu_sliders - handle the sliders menu
-------------------------------------------------*/

void ui_menu_sliders::handle()
{
	const ui_menu_event *menu_event;

	/* process the menu */
	menu_event = process(UI_MENU_PROCESS_LR_REPEAT | (hidden ? UI_MENU_PROCESS_CUSTOM_ONLY : 0));
	if (menu_event != NULL)
	{
		/* handle keys if there is a valid item selected */
		if (menu_event->itemref != NULL)
		{
			const slider_state *slider = (const slider_state *)menu_event->itemref;
			INT32 curvalue = (*slider->update)(machine(), slider->arg, NULL, SLIDER_NOCHANGE);
			INT32 increment = 0;

			switch (menu_event->iptkey)
			{
				/* toggle visibility */
				case IPT_UI_ON_SCREEN_DISPLAY:
					if (menuless_mode)
						ui_menu::stack_pop(machine());
					else
						hidden = !hidden;
					break;

				/* decrease value */
				case IPT_UI_LEFT:
					if (machine().input().code_pressed(KEYCODE_LALT) || machine().input().code_pressed(KEYCODE_RALT))
						increment = -1;
					else if (machine().input().code_pressed(KEYCODE_LSHIFT) || machine().input().code_pressed(KEYCODE_RSHIFT))
						increment = (slider->incval > 10) ? -(slider->incval / 10) : -1;
					else if (machine().input().code_pressed(KEYCODE_LCONTROL) || machine().input().code_pressed(KEYCODE_RCONTROL))
						increment = -slider->incval * 10;
					else
						increment = -slider->incval;
					break;

				/* increase value */
				case IPT_UI_RIGHT:
					if (machine().input().code_pressed(KEYCODE_LALT) || machine().input().code_pressed(KEYCODE_RALT))
						increment = 1;
					else if (machine().input().code_pressed(KEYCODE_LSHIFT) || machine().input().code_pressed(KEYCODE_RSHIFT))
						increment = (slider->incval > 10) ? (slider->incval / 10) : 1;
					else if (machine().input().code_pressed(KEYCODE_LCONTROL) || machine().input().code_pressed(KEYCODE_RCONTROL))
						increment = slider->incval * 10;
					else
						increment = slider->incval;
					break;

				/* restore default */
				case IPT_UI_SELECT:
					increment = slider->defval - curvalue;
					break;
			}

			/* handle any changes */
			if (increment != 0)
			{
				INT32 newvalue = curvalue + increment;

				/* clamp within bounds */
				if (newvalue < slider->minval)
					newvalue = slider->minval;
				if (newvalue > slider->maxval)
					newvalue = slider->maxval;

				/* update the slider and recompute the menu */
				(*slider->update)(machine(), slider->arg, NULL, newvalue);
				reset(UI_MENU_RESET_REMEMBER_REF);
			}
		}

		/* if we are selecting an invalid item and we are hidden, skip to the next one */
		else if (hidden)
		{
			/* if we got here via up or page up, select the previous item */
			if (menu_event->iptkey == IPT_UI_UP || menu_event->iptkey == IPT_UI_PAGE_UP)
			{
				selected = (selected + item.size() - 1) % item.size();
				validate_selection(-1);
			}

			/* otherwise select the next item */
			else if (menu_event->iptkey == IPT_UI_DOWN || menu_event->iptkey == IPT_UI_PAGE_DOWN)
			{
				selected = (selected + 1) % item.size();
				validate_selection(1);
			}
		}
	}
}


/*-------------------------------------------------
    menu_sliders_populate - populate the sliders
    menu
-------------------------------------------------*/

void ui_menu_sliders::populate()
{
	std::string tempstring;

	/* add UI sliders */
	for (const slider_state *curslider = machine().ui().get_slider_list(); curslider != NULL; curslider = curslider->next)
	{
		INT32 curval = (*curslider->update)(machine(), curslider->arg, &tempstring, SLIDER_NOCHANGE);
		UINT32 flags = 0;
		if (curval > curslider->minval)
			flags |= MENU_FLAG_LEFT_ARROW;
		if (curval < curslider->maxval)
			flags |= MENU_FLAG_RIGHT_ARROW;
		item_append(curslider->description, tempstring.c_str(), flags, (void *)curslider);
	}

	/* add OSD sliders */
	for (const slider_state *curslider = (slider_state*)machine().osd().get_slider_list(); curslider != NULL; curslider = curslider->next)
	{
		INT32 curval = (*curslider->update)(machine(), curslider->arg, &tempstring, SLIDER_NOCHANGE);
		UINT32 flags = 0;
		if (curval > curslider->minval)
			flags |= MENU_FLAG_LEFT_ARROW;
		if (curval < curslider->maxval)
			flags |= MENU_FLAG_RIGHT_ARROW;
		item_append(curslider->description, tempstring.c_str(), flags, (void *)curslider);
	}

	custombottom = 2.0f * machine().ui().get_line_height() + 2.0f * UI_BOX_TB_BORDER;
}

/*-------------------------------------------------
    menu_sliders_custom_render - perform our special
    rendering
-------------------------------------------------*/

void ui_menu_sliders::custom_render(void *selectedref, float top, float bottom, float x1, float y1, float x2, float y2)
{
	const slider_state *curslider = (const slider_state *)selectedref;
	if (curslider != NULL)
	{
		float bar_left, bar_area_top, bar_width, bar_area_height, bar_top, bar_bottom, default_x, current_x;
		float line_height = machine().ui().get_line_height();
		float percentage, default_percentage;
		std::string tempstring;
		float text_height;
		INT32 curval;

		/* determine the current value and text */
		curval = (*curslider->update)(machine(), curslider->arg, &tempstring, SLIDER_NOCHANGE);

		/* compute the current and default percentages */
		percentage = (float)(curval - curslider->minval) / (float)(curslider->maxval - curslider->minval);
		default_percentage = (float)(curslider->defval - curslider->minval) / (float)(curslider->maxval - curslider->minval);

		/* assemble the text */
		tempstring.insert(0, " ").insert(0, curslider->description);

		/* move us to the bottom of the screen, and expand to full width */
		y2 = 1.0f - UI_BOX_TB_BORDER;
		y1 = y2 - bottom;
		x1 = UI_BOX_LR_BORDER;
		x2 = 1.0f - UI_BOX_LR_BORDER;

		/* draw extra menu area */
		machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);
		y1 += UI_BOX_TB_BORDER;

		/* determine the text height */
		machine().ui().draw_text_full(container, tempstring.c_str(), 0, 0, x2 - x1 - 2.0f * UI_BOX_LR_BORDER,
					JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NONE, ARGB_WHITE, ARGB_BLACK, NULL, &text_height);

		/* draw the thermometer */
		bar_left = x1 + UI_BOX_LR_BORDER;
		bar_area_top = y1;
		bar_width = x2 - x1 - 2.0f * UI_BOX_LR_BORDER;
		bar_area_height = line_height;

		/* compute positions */
		bar_top = bar_area_top + 0.125f * bar_area_height;
		bar_bottom = bar_area_top + 0.875f * bar_area_height;
		default_x = bar_left + bar_width * default_percentage;
		current_x = bar_left + bar_width * percentage;

		/* fill in the percentage */
		container->add_rect(bar_left, bar_top, current_x, bar_bottom, UI_SLIDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		/* draw the top and bottom lines */
		container->add_line(bar_left, bar_top, bar_left + bar_width, bar_top, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container->add_line(bar_left, bar_bottom, bar_left + bar_width, bar_bottom, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		/* draw default marker */
		container->add_line(default_x, bar_area_top, default_x, bar_top, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container->add_line(default_x, bar_bottom, default_x, bar_area_top + bar_area_height, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		/* draw the actual text */
		machine().ui().draw_text_full(container, tempstring.c_str(), x1 + UI_BOX_LR_BORDER, y1 + line_height, x2 - x1 - 2.0f * UI_BOX_LR_BORDER,
					JUSTIFY_CENTER, WRAP_WORD, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, &text_height);
	}
}


/*-------------------------------------------------
 ui_slider_ui_handler - pushes the slider
 menu on the stack and hands off to the
 standard menu handler
 -------------------------------------------------*/

UINT32 ui_menu_sliders::ui_handler(running_machine &machine, render_container *container, UINT32 state)
{
	UINT32 result;

	/* if this is the first call, push the sliders menu */
	if (state)
		ui_menu::stack_push(auto_alloc_clear(machine, ui_menu_sliders(machine, container, true)));

	/* handle standard menus */
	result = ui_menu::ui_handler(machine, container, state);

	/* if we are cancelled, pop the sliders menu */
	if (result == UI_HANDLER_CANCEL)
		ui_menu::stack_pop(machine);

	ui_menu_sliders *uim = dynamic_cast<ui_menu_sliders *>(menu_stack);
	return uim && uim->menuless_mode ? 0 : UI_HANDLER_CANCEL;
}
