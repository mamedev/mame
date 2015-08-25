// license:BSD-3-Clause
// copyright-holders:Dankan1890
/*********************************************************************

    mewui/optsmenu.c

    MEWUI main options menu manager.

*********************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "mewui/datfile.h"
#include "mewui/inifile.h"
#include "mewui/utils.h"
#include "mewui/selector.h"
#include "mewui/custui.h"
#include "mewui/sndmenu.h"
#include "mewui/ctrlmenu.h"
#include "mewui/dsplmenu.h"
#include "mewui/miscmenu.h"
#include "mewui/optsmenu.h"
#include "mewui/custmenu.h"
#include "ui/inputmap.h"
#include "rendfont.h"

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_game_options::ui_menu_game_options(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_game_options::~ui_menu_game_options()
{
	ui_menu::menu_stack->reset(UI_MENU_RESET_SELECT_FIRST);
	save_game_options(machine());
	mewui_globals::switch_image = true;
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_game_options::handle()
{
	bool changed = false;

	// process the menu
//	ui_menu::menu_stack->parent->process(UI_MENU_PROCESS_NOINPUT);
//	const ui_menu_event *menu_event = process(UI_MENU_PROCESS_LR_REPEAT | UI_MENU_PROCESS_NOIMAGE);
	const ui_menu_event *menu_event = process(UI_MENU_PROCESS_LR_REPEAT);

	if (menu_event != NULL && menu_event->itemref != NULL)
		switch ((FPTR)menu_event->itemref)
		{
			case FILTER_MENU:
			{
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					(menu_event->iptkey == IPT_UI_RIGHT) ? mewui_globals::actual_filter++ : mewui_globals::actual_filter--;
					changed = true;
				}
				else if (menu_event->iptkey == IPT_UI_SELECT)
				{
					int total = mewui_globals::s_filter_text;
					std::vector<std::string> s_sel(total);
					for (int index = 0; index < total; index++)
						s_sel[index].assign(mewui_globals::filter_text[index]);

					ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, s_sel, &mewui_globals::actual_filter)));
				}
				break;
			}

			case FILE_CATEGORY_FILTER:
			{
				if (menu_event->iptkey == IPT_UI_LEFT)
				{
					machine().inifile().current_file--;
					machine().inifile().current_category = 0;
					changed = true;
				}
				else if (menu_event->iptkey == IPT_UI_RIGHT)
				{
					machine().inifile().current_file++;
					machine().inifile().current_category = 0;
					changed = true;
				}
				else if (menu_event->iptkey == IPT_UI_SELECT)
				{
					int total = machine().inifile().ini_index.size();
					std::vector<std::string> s_sel(total);
					machine().inifile().current_category = 0;
					for (size_t index = 0; index < total; ++index)
						s_sel[index].assign(machine().inifile().ini_index[index].name);

					ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, s_sel, &machine().inifile().current_file, SELECTOR_INIFILE)));
				}
				break;
			}

			case CATEGORY_FILTER:
			{
				if (menu_event->iptkey == IPT_UI_LEFT)
				{
					machine().inifile().current_category--;
					changed = true;
				}
				else if (menu_event->iptkey == IPT_UI_RIGHT)
				{
					machine().inifile().current_category++;
					changed = true;
				}
				else if (menu_event->iptkey == IPT_UI_SELECT)
				{
					int cfile = machine().inifile().current_file;
					int total = machine().inifile().ini_index[cfile].category.size();
					std::vector<std::string> s_sel(total);
					for (int index = 0; index < total; ++index)
						s_sel[index].assign(machine().inifile().ini_index[cfile].category[index].name);

					ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, s_sel, &machine().inifile().current_category, SELECTOR_CATEGORY)));
				}
				break;
			}

			case MANUFACT_CAT_FILTER:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					(menu_event->iptkey == IPT_UI_RIGHT) ? c_mnfct::actual++ : c_mnfct::actual--;
					changed = true;
				}
				else if (menu_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, c_mnfct::ui, &c_mnfct::actual)));

				break;

			case YEAR_CAT_FILTER:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					(menu_event->iptkey == IPT_UI_RIGHT) ? c_year::actual++ : c_year::actual--;
					changed = true;
				}
				else if (menu_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, c_year::ui, &c_year::actual)));

				break;

			case MISC_MENU:
				if (menu_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_misc_options(machine(), container)));
				break;

			case SOUND_MENU:
				if (menu_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_sound_options(machine(), container)));
				break;

			case DISPLAY_MENU:
				if (menu_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_display_options(machine(), container)));
				break;

			case CUSTOM_MENU:
				if (menu_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_custom_ui(machine(), container)));
				break;

			case CONTROLLER_MENU:
				if (menu_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_controller_mapping(machine(), container)));
				break;

			case CGI_MENU:
				if (menu_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_input_groups(machine(), container)));
				break;

			case CUSTOM_FILTER:
				if (menu_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_custom_filter(machine(), container)));
				break;

			case UME_SYSTEM:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					(menu_event->iptkey == IPT_UI_RIGHT) ? mewui_globals::ume_system++ : mewui_globals::ume_system--;
					changed = true;
				}
				break;
		}

	if (changed)
		reset(UI_MENU_RESET_REMEMBER_REF);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_game_options::populate()
{
	// set filter arrow
	std::string fbuff("^!File");

	// add filter item
	UINT32 arrow_flags = get_arrow_flags(0, mewui_globals::s_ume_text - 1, mewui_globals::ume_system);
	item_append("Machine", mewui_globals::ume_text[mewui_globals::ume_system], arrow_flags, (void *)UME_SYSTEM);

	// add filter item
	arrow_flags = get_arrow_flags((int)FILTER_FIRST, (int)FILTER_LAST, mewui_globals::actual_filter);
	item_append("Filter", mewui_globals::filter_text[mewui_globals::actual_filter], arrow_flags, (void *)FILTER_MENU);

	// add category subitem
	if (mewui_globals::actual_filter == FILTER_CATEGORY && !machine().inifile().ini_index.empty())
	{
		int actual_file = machine().inifile().current_file;

		arrow_flags = get_arrow_flags(0, machine().inifile().ini_index.size() - 1, actual_file);
		convert_command_glyph(fbuff);
		item_append(fbuff.c_str(), machine().inifile().ini_index[actual_file].name.c_str(), arrow_flags, (void *)FILE_CATEGORY_FILTER);

		int actual_category = machine().inifile().current_category;
		arrow_flags = get_arrow_flags(0, machine().inifile().ini_index[actual_file].category.size() - 1, actual_category);
		fbuff.assign(" ^!Category");
		convert_command_glyph(fbuff);
		item_append(fbuff.c_str(), machine().inifile().ini_index[actual_file].category[actual_category].name.c_str(), arrow_flags, (void *)CATEGORY_FILTER);
	}
	// add manufacturer subitem
	else if (mewui_globals::actual_filter == FILTER_MANUFACTURER && c_mnfct::ui.size() > 0)
	{
		arrow_flags = get_arrow_flags(0, c_mnfct::ui.size() - 1, c_mnfct::actual);
		fbuff.assign("^!Manufacturer");
		convert_command_glyph(fbuff);
		item_append(fbuff.c_str(), c_mnfct::ui[c_mnfct::actual].c_str(), arrow_flags, (void *)MANUFACT_CAT_FILTER);
	}
	// add year subitem
	else if (mewui_globals::actual_filter == FILTER_YEAR && c_year::ui.size() > 0)
	{
		arrow_flags = get_arrow_flags(0, c_year::ui.size() - 1, c_year::actual);
		fbuff.assign("^!Year");
		convert_command_glyph(fbuff);
		item_append(fbuff.c_str(), c_year::ui[c_year::actual].c_str(), arrow_flags, (void *)YEAR_CAT_FILTER);
	}
	// add custom subitem
	else if (mewui_globals::actual_filter == FILTER_CUSTOM)
	{
		fbuff.assign("^!Setup custom filter");
		convert_command_glyph(fbuff);
		item_append(fbuff.c_str(), NULL, 0, (void *)CUSTOM_FILTER);
	}

	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);

	// add options items
	item_append("Customize UI", NULL, 0, (void *)CUSTOM_MENU);
	item_append("Display Options", NULL, 0, (void *)DISPLAY_MENU);
	item_append("Sound Options", NULL, 0, (void *)SOUND_MENU);
	item_append("Miscellaneous Options", NULL, 0, (void *)MISC_MENU);
	item_append("Device Mapping", NULL, 0, (void *)CONTROLLER_MENU);
	item_append("General Inputs", NULL, 0, (void *)CGI_MENU);
	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);

	custombottom = 2.0f * machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
	customtop = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_game_options::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;
	machine().ui().draw_text_full(container, "Settings", 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
	                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
	width += 2 * UI_BOX_LR_BORDER;
	float maxwidth = MAX(origx2 - origx1, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	machine().ui().draw_text_full(container, "Settings", x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
}

//-------------------------------------------------
//  save game options
//-------------------------------------------------

void save_game_options(running_machine &machine)
{
	// attempt to open the output file
	emu_file file(machine.options().ini_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	if (file.open(emulator_info::get_configname(), ".ini") == FILERR_NONE)
	{
		// generate the updated INI
		std::string initext;
		file.puts(machine.options().output_ini(initext));
		file.close();
	}
	else
		popmessage("**Error to save %s.ini**", emulator_info::get_configname());
}
