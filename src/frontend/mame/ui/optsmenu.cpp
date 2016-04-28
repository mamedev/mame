// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/*********************************************************************

    ui/optsmenu.cpp

    UI main options menu manager.

*********************************************************************/

#include "emu.h"
#include "mame.h"
#include "mameopts.h"
#include "ui/ui.h"
#include "ui/menu.h"
#include "ui/submenu.h"
#include "ui/inifile.h"
#include "ui/selector.h"
#include "ui/custui.h"
#include "ui/sndmenu.h"
#include "ui/optsmenu.h"
#include "ui/custmenu.h"
#include "ui/inputmap.h"
#include "ui/dirmenu.h"
#include "rendfont.h"

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_game_options::ui_menu_game_options(mame_ui_manager &mui, render_container *container) : ui_menu(mui, container)
{
	m_main = main_filters::actual;
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_game_options::~ui_menu_game_options()
{
	main_filters::actual = m_main;
	if (ui_menu::menu_stack != nullptr)
		ui_menu::menu_stack->reset(UI_MENU_RESET_SELECT_FIRST);
	ui().save_ui_options();
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

					ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(ui(), container, s_sel, m_main));
				}
				break;
			}
			case FILE_CATEGORY_FILTER:
			{
				if (m_event->iptkey == IPT_UI_LEFT)
				{
					mame_machine_manager::instance()->inifile().move_file(-1);
					changed = true;
				}
				else if (m_event->iptkey == IPT_UI_RIGHT)
				{
					mame_machine_manager::instance()->inifile().move_file(1);
					changed = true;
				}
				else if (m_event->iptkey == IPT_UI_SELECT)
				{
					inifile_manager &ifile = mame_machine_manager::instance()->inifile();
					int total = ifile.total();
					std::vector<std::string> s_sel(total);
					mame_machine_manager::instance()->inifile().set_cat(0);
					for (size_t index = 0; index < total; ++index)
						s_sel[index] = ifile.get_file(index);

					ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(ui(), container, s_sel, ifile.cur_file(), SELECTOR_INIFILE));
				}
				break;
			}
			case CATEGORY_FILTER:
			{
				if (m_event->iptkey == IPT_UI_LEFT)
				{
					mame_machine_manager::instance()->inifile().move_cat(-1);
					changed = true;
				}
				else if (m_event->iptkey == IPT_UI_RIGHT)
				{
					mame_machine_manager::instance()->inifile().move_cat(1);
					changed = true;
				}
				else if (m_event->iptkey == IPT_UI_SELECT)
				{
					inifile_manager &ifile = mame_machine_manager::instance()->inifile();
					int total = ifile.cat_total();
					std::vector<std::string> s_sel(total);
					for (int index = 0; index < total; ++index)
						s_sel[index] = ifile.get_category(index);

					ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(ui(), container, s_sel, ifile.cur_cat(), SELECTOR_CATEGORY));
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
					ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(ui(), container, c_mnfct::ui, c_mnfct::actual));

				break;
			case YEAR_CAT_FILTER:
				if (m_event->iptkey == IPT_UI_LEFT || m_event->iptkey == IPT_UI_RIGHT)
				{
					(m_event->iptkey == IPT_UI_RIGHT) ? c_year::actual++ : c_year::actual--;
					changed = true;
				}
				else if (m_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(ui(), container, c_year::ui, c_year::actual));

				break;
			case CONF_DIR:
				if (m_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(global_alloc_clear<ui_menu_directory>(ui(), container));
				break;
			case MISC_MENU:
				if (m_event->iptkey == IPT_UI_SELECT)
				{
					ui_menu::stack_push(global_alloc_clear<ui_submenu>(ui(), container, misc_submenu_options));
					ui_globals::reset = true;
				}
				break;
			case SOUND_MENU:
				if (m_event->iptkey == IPT_UI_SELECT)
				{
					ui_menu::stack_push(global_alloc_clear<ui_menu_sound_options>(ui(), container));
					ui_globals::reset = true;
				}
				break;
			case DISPLAY_MENU:
				if (m_event->iptkey == IPT_UI_SELECT)
				{
					ui_menu::stack_push(global_alloc_clear<ui_submenu>(ui(), container, video_submenu_options));
					ui_globals::reset = true;
				}
				break;
			case CUSTOM_MENU:
				if (m_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(global_alloc_clear<ui_menu_custom_ui>(ui(), container));
				break;
			case CONTROLLER_MENU:
				if (m_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(global_alloc_clear<ui_submenu>(ui(), container, control_submenu_options));
				break;
			case CGI_MENU:
				if (m_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(global_alloc_clear<ui_menu_input_groups>(ui(), container));
				break;
			case CUSTOM_FILTER:
				if (m_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(global_alloc_clear<ui_menu_custom_filter>(ui(), container));
				break;
			case ADVANCED_MENU:
				if (m_event->iptkey == IPT_UI_SELECT)
				{
					ui_menu::stack_push(global_alloc_clear<ui_submenu>(ui(), container, advanced_submenu_options));
					ui_globals::reset = true;
				}
				break;
			case SAVE_CONFIG:
				if (m_event->iptkey == IPT_UI_SELECT)
					ui().save_main_option();
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
		if (m_main == FILTER_CATEGORY && mame_machine_manager::instance()->inifile().total() > 0)
		{
			inifile_manager &inif = mame_machine_manager::instance()->inifile();

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

		item_append(ui_menu_item_type::SEPARATOR);

		// add options items
		item_append(_("Customize UI"), nullptr, 0, (void *)(FPTR)CUSTOM_MENU);
		item_append(_("Configure Directories"), nullptr, 0, (void *)(FPTR)CONF_DIR);
	}
	item_append(_(video_submenu_options[0].description), nullptr, 0, (void *)(FPTR)DISPLAY_MENU);
	item_append(_("Sound Options"), nullptr, 0, (void *)(FPTR)SOUND_MENU);
	item_append(_(misc_submenu_options[0].description), nullptr, 0, (void *)(FPTR)MISC_MENU);
	item_append(_(control_submenu_options[0].description), nullptr, 0, (void *)(FPTR)CONTROLLER_MENU);
	item_append(_("General Inputs"), nullptr, 0, (void *)(FPTR)CGI_MENU);
	item_append(_(advanced_submenu_options[0].description), nullptr, 0, (void *)(FPTR)ADVANCED_MENU);
	item_append(ui_menu_item_type::SEPARATOR);
	item_append(_("Save Configuration"), nullptr, 0, (void *)(FPTR)SAVE_CONFIG);

	custombottom = 2.0f * ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
	customtop = ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_game_options::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;
	ui().draw_text_full(container, _("Settings"), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
									DRAW_NONE, rgb_t::white, rgb_t::black, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	float maxwidth = MAX(origx2 - origx1, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	ui().draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	ui().draw_text_full(container, _("Settings"), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
									DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
}
