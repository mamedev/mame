// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/*********************************************************************

    ui/optsmenu.cpp

    UI main options menu manager.

*********************************************************************/

#include "emu.h"
#include "ui/optsmenu.h"

#include "ui/ui.h"
#include "ui/submenu.h"
#include "ui/selector.h"
#include "ui/custui.h"
#include "ui/sndmenu.h"
#include "ui/inputmap.h"
#include "ui/dirmenu.h"

#include "mame.h"
#include "mameopts.h"
#include "rendfont.h"


namespace ui {

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_game_options::menu_game_options(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
	m_main = main_filters::actual;
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_game_options::~menu_game_options()
{
	main_filters::actual = m_main;
	reset_topmost(reset_options::SELECT_FIRST);
	ui().save_ui_options();
	ui_globals::switch_image = true;
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_game_options::handle()
{
	bool changed = false;

	// process the menu
	const event *menu_event;
	if (machine().options().ui() == emu_options::UI_SIMPLE)
	{
		menu_event = process(PROCESS_LR_REPEAT);
	}
	else
	{
		process_parent();
		menu_event = process(PROCESS_LR_REPEAT | PROCESS_NOIMAGE);
	}

	if (menu_event != nullptr && menu_event->itemref != nullptr)
		switch ((uintptr_t)menu_event->itemref)
		{
		case FILTER_MENU:
			if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
			{
				(menu_event->iptkey == IPT_UI_RIGHT) ? ++m_main : --m_main;
				changed = true;
			}
			else if (menu_event->iptkey == IPT_UI_SELECT)
			{
				std::vector<std::string> s_sel(machine_filter::COUNT);
				for (unsigned index = 0; index < s_sel.size(); ++index)
					s_sel[index] = machine_filter::display_name(machine_filter::type(index));

				menu::stack_push<menu_selector>(
						ui(), container(), std::move(s_sel), m_main,
						[this] (int selection)
						{
							m_main = machine_filter::type(selection);
							reset(reset_options::REMEMBER_REF);
						});
			}
			break;
		case FILTER_ADJUST:
			if (menu_event->iptkey == IPT_UI_LEFT)
			{
				changed = main_filters::filters.find(m_main)->second->adjust_left();
			}
			else if (menu_event->iptkey == IPT_UI_RIGHT)
			{
				changed = main_filters::filters.find(m_main)->second->adjust_right();
			}
			else if (menu_event->iptkey == IPT_UI_SELECT)
			{
				main_filters::filters.find(m_main)->second->show_ui(
						ui(),
						container(),
						[this] (machine_filter &filter)
						{
							if (machine_filter::CUSTOM == filter.get_type())
							{
								emu_file file(ui().options().ui_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
								if (file.open("custom_", emulator_info::get_configname(), "_filter.ini") == osd_file::error::NONE)
								{
									filter.save_ini(file, 0);
									file.close();
								}
							}
							reset(reset_options::REMEMBER_REF);
						});
			}
			break;
		case CONF_DIR:
			if (menu_event->iptkey == IPT_UI_SELECT)
				menu::stack_push<menu_directory>(ui(), container());
			break;
		case MISC_MENU:
			if (menu_event->iptkey == IPT_UI_SELECT)
			{
				menu::stack_push<submenu>(ui(), container(), submenu::misc_options);
				ui_globals::reset = true;
			}
			break;
		case SOUND_MENU:
			if (menu_event->iptkey == IPT_UI_SELECT)
			{
				menu::stack_push<menu_sound_options>(ui(), container());
				ui_globals::reset = true;
			}
			break;
		case DISPLAY_MENU:
			if (menu_event->iptkey == IPT_UI_SELECT)
			{
				menu::stack_push<submenu>(ui(), container(), submenu::video_options);
				ui_globals::reset = true;
			}
			break;
		case CUSTOM_MENU:
			if (menu_event->iptkey == IPT_UI_SELECT)
				menu::stack_push<menu_custom_ui>(ui(), container());
			break;
		case CONTROLLER_MENU:
			if (menu_event->iptkey == IPT_UI_SELECT)
				menu::stack_push<submenu>(ui(), container(), submenu::control_options);
			break;
		case CGI_MENU:
			if (menu_event->iptkey == IPT_UI_SELECT)
				menu::stack_push<menu_input_groups>(ui(), container());
			break;
		case ADVANCED_MENU:
			if (menu_event->iptkey == IPT_UI_SELECT)
			{
				menu::stack_push<submenu>(ui(), container(), submenu::advanced_options);
				ui_globals::reset = true;
			}
			break;
		case SAVE_CONFIG:
			if (menu_event->iptkey == IPT_UI_SELECT)
				ui().save_main_option();
			break;
		}

	if (changed)
		reset(reset_options::REMEMBER_REF);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_game_options::populate(float &customtop, float &custombottom)
{
	if (machine().options().ui() != emu_options::UI_SIMPLE)
	{
		// set filter arrow
		std::string fbuff;

		// add filter item
		uint32_t arrow_flags = get_arrow_flags<uint16_t>(machine_filter::FIRST, machine_filter::LAST, m_main);
		auto active_filter(main_filters::filters.find(m_main));
		if (main_filters::filters.end() == active_filter)
			active_filter = main_filters::filters.emplace(m_main, machine_filter::create(m_main)).first;
		item_append(_("Filter"), active_filter->second->display_name(), arrow_flags, (void *)(uintptr_t)FILTER_MENU);

		// add subitem if the filter wants it
		if (active_filter->second->wants_adjuster())
		{
			std::string name("^!");
			convert_command_glyph(name);
			item_append(name, active_filter->second->adjust_text(), active_filter->second->arrow_flags(), (void *)(FILTER_ADJUST));
		}

		item_append(menu_item_type::SEPARATOR);

		// add options items
		item_append(_("Customize UI"), "", 0, (void *)(uintptr_t)CUSTOM_MENU);
		item_append(_("Configure Directories"), "", 0, (void *)(uintptr_t)CONF_DIR);
	}
	item_append(_(submenu::video_options[0].description), "", 0, (void *)(uintptr_t)DISPLAY_MENU);
	item_append(_("Sound Options"), "", 0, (void *)(uintptr_t)SOUND_MENU);
	item_append(_(submenu::misc_options[0].description), "", 0, (void *)(uintptr_t)MISC_MENU);
	item_append(_(submenu::control_options[0].description), "", 0, (void *)(uintptr_t)CONTROLLER_MENU);
	item_append(_("General Inputs"), "", 0, (void *)(uintptr_t)CGI_MENU);
	item_append(_(submenu::advanced_options[0].description), "", 0, (void *)(uintptr_t)ADVANCED_MENU);
	item_append(menu_item_type::SEPARATOR);
	item_append(_("Save Configuration"), "", 0, (void *)(uintptr_t)SAVE_CONFIG);

	custombottom = 2.0f * ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
	customtop = ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_game_options::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	char const *const toptext[] = { _("Settings") };
	draw_text_box(
			std::begin(toptext), std::end(toptext),
			origx1, origx2, origy1 - top, origy1 - UI_BOX_TB_BORDER,
			ui::text_layout::CENTER, ui::text_layout::TRUNCATE, false,
			UI_TEXT_COLOR, UI_GREEN_COLOR, 1.0f);
}


} // namespace ui
