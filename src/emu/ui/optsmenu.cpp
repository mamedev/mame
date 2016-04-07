// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/*********************************************************************

    ui/optsmenu.cpp

    UI main options menu manager.

*********************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "ui/menu.h"
#include "ui/submenu.h"
#include "ui/datfile.h"
#include "ui/inifile.h"
#include "ui/selector.h"
#include "ui/custui.h"
#include "ui/sndmenu.h"
#include "ui/ctrlmenu.h"
#include "ui/dsplmenu.h"
#include "ui/miscmenu.h"
#include "ui/optsmenu.h"
#include "ui/custmenu.h"
#include "ui/inputmap.h"
#include "ui/dirmenu.h"
#include "rendfont.h"

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_game_options::ui_menu_game_options(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
	m_main = main_filters::actual;
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_game_options::~ui_menu_game_options()
{
	main_filters::actual = m_main;
	ui_menu::menu_stack->reset(UI_MENU_RESET_SELECT_FIRST);
	save_ui_options(machine());
	ui_globals::switch_image = true;
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_game_options::handle()
{
	bool changed = false;

	// process the menu
	const ui_menu_event *m_event;
	if (strcmp(machine().options().ui(), "simple") == 0)
	{
		m_event = process(UI_MENU_PROCESS_LR_REPEAT);
	}
	else
	{
		ui_menu::menu_stack->parent->process(UI_MENU_PROCESS_NOINPUT);
		m_event = process(UI_MENU_PROCESS_LR_REPEAT | UI_MENU_PROCESS_NOIMAGE);
	}

	if (m_event != nullptr && m_event->itemref != nullptr)
		switch ((FPTR)m_event->itemref)
		{
			case FILTER_MENU:
			{
				if (m_event->iptkey == IPT_UI_LEFT || m_event->iptkey == IPT_UI_RIGHT)
				{
					(m_event->iptkey == IPT_UI_RIGHT) ? ++m_main : --m_main;
					changed = true;
				}
				else if (m_event->iptkey == IPT_UI_SELECT)
				{
					int total = main_filters::length;
					std::vector<std::string> s_sel(total);
					for (int index = 0; index < total; ++index)
						s_sel[index] = main_filters::text[index];

					ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(machine(), container, s_sel, m_main));
				}
				break;
			}
			case FILE_CATEGORY_FILTER:
			{
				if (m_event->iptkey == IPT_UI_LEFT)
				{
					machine().inifile().move_file(-1);
					changed = true;
				}
				else if (m_event->iptkey == IPT_UI_RIGHT)
				{
					machine().inifile().move_file(1);
					changed = true;
				}
				else if (m_event->iptkey == IPT_UI_SELECT)
				{
					inifile_manager &ifile = machine().inifile();
					int total = ifile.total();
					std::vector<std::string> s_sel(total);
					machine().inifile().set_cat(0);
					for (size_t index = 0; index < total; ++index)
						s_sel[index] = ifile.get_file(index);

					ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(machine(), container, s_sel, ifile.cur_file(), SELECTOR_INIFILE));
				}
				break;
			}
			case CATEGORY_FILTER:
			{
				if (m_event->iptkey == IPT_UI_LEFT)
				{
					machine().inifile().move_cat(-1);
					changed = true;
				}
				else if (m_event->iptkey == IPT_UI_RIGHT)
				{
					machine().inifile().move_cat(1);
					changed = true;
				}
				else if (m_event->iptkey == IPT_UI_SELECT)
				{
					inifile_manager &ifile = machine().inifile();
					int total = ifile.cat_total();
					std::vector<std::string> s_sel(total);
					for (int index = 0; index < total; ++index)
						s_sel[index] = ifile.get_category(index);

					ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(machine(), container, s_sel, ifile.cur_cat(), SELECTOR_CATEGORY));
				}
				break;
			}
			case MANUFACT_CAT_FILTER:
				if (m_event->iptkey == IPT_UI_LEFT || m_event->iptkey == IPT_UI_RIGHT)
				{
					(m_event->iptkey == IPT_UI_RIGHT) ? c_mnfct::actual++ : c_mnfct::actual--;
					changed = true;
				}
				else if (m_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(machine(), container, c_mnfct::ui, c_mnfct::actual));

				break;
			case YEAR_CAT_FILTER:
				if (m_event->iptkey == IPT_UI_LEFT || m_event->iptkey == IPT_UI_RIGHT)
				{
					(m_event->iptkey == IPT_UI_RIGHT) ? c_year::actual++ : c_year::actual--;
					changed = true;
				}
				else if (m_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(machine(), container, c_year::ui, c_year::actual));

				break;
			case CONF_DIR:
				if (m_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(global_alloc_clear<ui_menu_directory>(machine(), container));
				break;
			case MISC_MENU:
				if (m_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(global_alloc_clear<ui_submenu>(machine(), container, misc_submenu_options));
				break;
			case SOUND_MENU:
				if (m_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(global_alloc_clear<ui_menu_sound_options>(machine(), container));
				break;
			case DISPLAY_MENU:
				if (m_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(global_alloc_clear<ui_menu_display_options>(machine(), container));
				break;
			case CUSTOM_MENU:
				if (m_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(global_alloc_clear<ui_menu_custom_ui>(machine(), container));
				break;
			case CONTROLLER_MENU:
				if (m_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(global_alloc_clear<ui_menu_controller_mapping>(machine(), container));
				break;
			case CGI_MENU:
				if (m_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(global_alloc_clear<ui_menu_input_groups>(machine(), container));
				break;
			case CUSTOM_FILTER:
				if (m_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(global_alloc_clear<ui_menu_custom_filter>(machine(), container));
				break;
			case ADVANCED_MENU:
				if (m_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(global_alloc_clear<ui_submenu>(machine(), container, advanced_submenu_options));
				break;
			case SAVE_CONFIG:
				if (m_event->iptkey == IPT_UI_SELECT)
					save_main_option(machine());
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
	if (strcmp(machine().options().ui(),"simple")!=0)
	{
		// set filter arrow
		std::string fbuff;

		// add filter item
		UINT32 arrow_flags = get_arrow_flags((int)FILTER_FIRST, (int)FILTER_LAST, m_main);
		item_append(_("Filter"), main_filters::text[m_main], arrow_flags, (void *)(FPTR)FILTER_MENU);

		// add category subitem
		if (m_main == FILTER_CATEGORY && machine().inifile().total() > 0)
		{
			inifile_manager &inif = machine().inifile();

			arrow_flags = get_arrow_flags(0, inif.total() - 1, inif.cur_file());
			fbuff = _(" ^!File");
			convert_command_glyph(fbuff);
			item_append(fbuff.c_str(), inif.get_file().c_str(), arrow_flags, (void *)(FPTR)FILE_CATEGORY_FILTER);

			arrow_flags = get_arrow_flags(0, inif.cat_total() - 1, inif.cur_cat());
			fbuff = _(" ^!Category");
			convert_command_glyph(fbuff);
			item_append(fbuff.c_str(), inif.get_category().c_str(), arrow_flags, (void *)(FPTR)CATEGORY_FILTER);
		}
		// add manufacturer subitem
		else if (m_main == FILTER_MANUFACTURER && c_mnfct::ui.size() > 0)
		{
			arrow_flags = get_arrow_flags(0, c_mnfct::ui.size() - 1, c_mnfct::actual);
			fbuff = _("^!Manufacturer");
			convert_command_glyph(fbuff);
			item_append(fbuff.c_str(), c_mnfct::ui[c_mnfct::actual].c_str(), arrow_flags, (void *)(FPTR)MANUFACT_CAT_FILTER);
		}
		// add year subitem
		else if (m_main == FILTER_YEAR && c_year::ui.size() > 0)
		{
			arrow_flags = get_arrow_flags(0, c_year::ui.size() - 1, c_year::actual);
			fbuff.assign(_("^!Year"));
			convert_command_glyph(fbuff);
			item_append(fbuff.c_str(), c_year::ui[c_year::actual].c_str(), arrow_flags, (void *)(FPTR)YEAR_CAT_FILTER);
		}
		// add custom subitem
		else if (m_main == FILTER_CUSTOM)
		{
			fbuff = _("^!Setup custom filter");
			convert_command_glyph(fbuff);
			item_append(fbuff.c_str(), nullptr, 0, (void *)(FPTR)CUSTOM_FILTER);
		}

		item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);

		// add options items
		item_append(_("Customize UI"), nullptr, 0, (void *)(FPTR)CUSTOM_MENU);
		item_append(_("Configure Directories"), nullptr, 0, (void *)(FPTR)CONF_DIR);
	}
	item_append(_("Display Options"), nullptr, 0, (void *)(FPTR)DISPLAY_MENU);
	item_append(_("Sound Options"), nullptr, 0, (void *)(FPTR)SOUND_MENU);
	item_append(_(misc_submenu_options[0].description),    nullptr, 0, (void *)(FPTR)MISC_MENU);
	item_append(_("Device Mapping"), nullptr, 0, (void *)(FPTR)CONTROLLER_MENU);
	item_append(_("General Inputs"), nullptr, 0, (void *)(FPTR)CGI_MENU);
	item_append(_(advanced_submenu_options[0].description),  nullptr, 0, (void *)(FPTR)ADVANCED_MENU);
	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
	item_append(_("Save Configuration"), nullptr, 0, (void *)(FPTR)SAVE_CONFIG);

	custombottom = 2.0f * machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
	customtop = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_game_options::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;
	ui_manager &mui = machine().ui();
	mui.draw_text_full(container, _("Settings"), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
									DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	float maxwidth = MAX(origx2 - origx1, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	mui.draw_text_full(container, _("Settings"), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
									DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
}

//-------------------------------------------------
//  save ui options
//-------------------------------------------------

void save_ui_options(running_machine &machine)
{
	// attempt to open the output file
	emu_file file(machine.options().ini_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	if (file.open("ui.ini") == osd_file::error::NONE)
	{
		// generate the updated INI
		std::string initext = machine.ui().options().output_ini();
		file.puts(initext.c_str());
		file.close();
	}
	else
		machine.popmessage(_("**Error saving ui.ini**"));
}

//-------------------------------------------------
//  save main option
//-------------------------------------------------

void save_main_option(running_machine &machine)
{
	// parse the file
	std::string error;
	emu_options options(machine.options()); // This way we make sure that all OSD parts are in
	std::string error_string;

	// attempt to open the main ini file
	{
		emu_file file(machine.options().ini_path(), OPEN_FLAG_READ);
		if (file.open(emulator_info::get_configname(), ".ini") == osd_file::error::NONE)
		{
			bool result = options.parse_ini_file((util::core_file&)file, OPTION_PRIORITY_MAME_INI, OPTION_PRIORITY_DRIVER_INI, error);
			if (!result)
			{
				osd_printf_error("**Error loading %s.ini**", emulator_info::get_configname());
				return;
			}
		}
	}

	for (emu_options::entry &f_entry : machine.options())
	{
		if (f_entry.is_changed())
		{
			options.set_value(f_entry.name(), f_entry.value(), OPTION_PRIORITY_CMDLINE, error_string);
		}
	}

	// attempt to open the output file
	{
		emu_file file(machine.options().ini_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
		if (file.open(emulator_info::get_configname(), ".ini") == osd_file::error::NONE)
		{
			// generate the updated INI
			std::string initext = options.output_ini();
			file.puts(initext.c_str());
			file.close();
		}
		else {
			machine.popmessage(_("**Error saving %s.ini**"), emulator_info::get_configname());
			return;
		}
	}
	machine.ui().popup_time(3, "%s", _("\n    Configuration saved    \n\n"));
}
