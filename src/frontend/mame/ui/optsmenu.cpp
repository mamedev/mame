// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/*********************************************************************

    ui/optsmenu.cpp

    UI main options menu manager.

*********************************************************************/

#include "emu.h"
#include "ui/optsmenu.h"

#include "ui/custui.h"
#include "ui/dirmenu.h"
#include "ui/inputmap.h"
#include "ui/miscmenu.h"
#include "ui/selector.h"
#include "ui/sndmenu.h"
#include "ui/submenu.h"
#include "ui/ui.h"

#include "mame.h"
#include "mameopts.h"

#include "fileio.h"
#include "rendfont.h"


namespace ui {

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_simple_game_options::menu_simple_game_options(
		mame_ui_manager &mui,
		render_container &container,
		std::function<void ()> &&handler)
	: menu(mui, container)
	, m_handler(std::move(handler))
{
	set_process_flags(PROCESS_LR_REPEAT);
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_simple_game_options::~menu_simple_game_options()
{
	ui().save_ui_options();
	if (m_handler)
		m_handler();
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_simple_game_options::handle(event const *ev)
{
	// process the menu
	if (ev && ev->itemref)
		handle_item_event(*ev);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_simple_game_options::populate(float &customtop, float &custombottom)
{
	item_append(_(submenu::video_options()[0].description), 0, (void *)(uintptr_t)DISPLAY_MENU);
	item_append(_("Sound Options"), 0, (void *)(uintptr_t)SOUND_MENU);
	item_append(_(submenu::misc_options()[0].description), 0, (void *)(uintptr_t)MISC_MENU);
	item_append(_(submenu::control_options()[0].description), 0, (void *)(uintptr_t)CONTROLLER_MENU);
	item_append(_("General Inputs"), 0, (void *)(uintptr_t)CGI_MENU);
	item_append(_(submenu::advanced_options()[0].description), 0, (void *)(uintptr_t)ADVANCED_MENU);
	if (machine().options().plugins())
		item_append(_("Plugins"), 0, (void *)(uintptr_t)PLUGINS_MENU);
	item_append(menu_item_type::SEPARATOR);
	item_append(_("Save Configuration"), 0, (void *)(uintptr_t)SAVE_CONFIG);

	custombottom = 2.0f * ui().get_line_height() + 3.0f * ui().box_tb_border();
	customtop = ui().get_line_height() + 3.0f * ui().box_tb_border();
}

//-------------------------------------------------
//  handle item
//-------------------------------------------------

void menu_simple_game_options::handle_item_event(event const &menu_event)
{
	switch ((uintptr_t)menu_event.itemref)
	{
	case MISC_MENU:
		if (menu_event.iptkey == IPT_UI_SELECT)
		{
			menu::stack_push<submenu>(ui(), container(), submenu::misc_options());
			ui_globals::reset = true;
		}
		break;
	case SOUND_MENU:
		if (menu_event.iptkey == IPT_UI_SELECT)
		{
			menu::stack_push<menu_sound_options>(ui(), container());
			ui_globals::reset = true;
		}
		break;
	case DISPLAY_MENU:
		if (menu_event.iptkey == IPT_UI_SELECT)
		{
			menu::stack_push<submenu>(ui(), container(), submenu::video_options());
			ui_globals::reset = true;
		}
		break;
	case CONTROLLER_MENU:
		if (menu_event.iptkey == IPT_UI_SELECT)
			menu::stack_push<submenu>(ui(), container(), submenu::control_options());
		break;
	case CGI_MENU:
		if (menu_event.iptkey == IPT_UI_SELECT)
			menu::stack_push<menu_input_groups>(ui(), container());
		break;
	case ADVANCED_MENU:
		if (menu_event.iptkey == IPT_UI_SELECT)
		{
			menu::stack_push<submenu>(ui(), container(), submenu::advanced_options());
			ui_globals::reset = true;
		}
		break;
	case PLUGINS_MENU:
		if (menu_event.iptkey == IPT_UI_SELECT)
			menu::stack_push<menu_plugins_configure>(ui(), container());
		break;
	case SAVE_CONFIG:
		if (menu_event.iptkey == IPT_UI_SELECT)
			ui().save_main_option();
		break;
	}
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_simple_game_options::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	char const *const toptext[] = { _("Settings") };
	draw_text_box(
			std::begin(toptext), std::end(toptext),
			origx1, origx2, origy1 - top, origy1 - ui().box_tb_border(),
			text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE, false,
			ui().colors().text_color(), UI_GREEN_COLOR, 1.0f);
}


//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_game_options::menu_game_options(
		mame_ui_manager &mui,
		render_container &container,
		machine_filter_data &filter_data,
		std::function<void ()> &&handler)
	: menu_simple_game_options(mui, container, std::move(handler))
	, m_filter_data(filter_data)
	, m_main_filter(filter_data.get_current_filter_type())
{
	set_process_flags(PROCESS_LR_REPEAT);
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_game_options::~menu_game_options()
{
	m_filter_data.set_current_filter_type(m_main_filter);
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_game_options::handle(event const *ev)
{
	// process the menu
	if (ev && ev->itemref)
		handle_item_event(*ev);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_game_options::populate(float &customtop, float &custombottom)
{
	// set filter arrow
	std::string fbuff;

	// add filter item
	uint32_t arrow_flags = get_arrow_flags<uint16_t>(machine_filter::FIRST, machine_filter::LAST, m_main_filter);
	machine_filter &active_filter(m_filter_data.get_filter(m_main_filter));
	item_append(_("Filter"), active_filter.display_name(), arrow_flags, (void *)(uintptr_t)FILTER_MENU);

	// add subitem if the filter wants it
	if (active_filter.wants_adjuster())
	{
		std::string name(convert_command_glyph("^!"));
		item_append(name, active_filter.adjust_text(), active_filter.arrow_flags(), (void *)(FILTER_ADJUST));
	}

	item_append(menu_item_type::SEPARATOR);

	// add options items
	item_append(_("Customize UI"), 0, (void *)(uintptr_t)CUSTOM_MENU);
	item_append(_("Configure Directories"), 0, (void *)(uintptr_t)CONF_DIR);

	// add the options that don't relate to the UI
	menu_simple_game_options::populate(customtop, custombottom);
}

//-------------------------------------------------
//  handle item
//-------------------------------------------------

void menu_game_options::handle_item_event(event const &menu_event)
{
	bool changed = false;

	switch ((uintptr_t)menu_event.itemref)
	{
	case FILTER_MENU:
		if (menu_event.iptkey == IPT_UI_LEFT || menu_event.iptkey == IPT_UI_RIGHT)
		{
			(menu_event.iptkey == IPT_UI_RIGHT) ? ++m_main_filter : --m_main_filter;
			changed = true;
		}
		else if (menu_event.iptkey == IPT_UI_SELECT)
		{
			std::vector<std::string> s_sel(machine_filter::COUNT);
			for (unsigned index = 0; index < s_sel.size(); ++index)
				s_sel[index] = machine_filter::display_name(machine_filter::type(index));

			menu::stack_push<menu_selector>(
					ui(), container(), std::move(s_sel), m_main_filter,
					[this] (int selection)
					{
						m_main_filter = machine_filter::type(selection);
						reset(reset_options::REMEMBER_REF);
					});
		}
		break;
	case FILTER_ADJUST:
		if (menu_event.iptkey == IPT_UI_LEFT)
		{
			changed = m_filter_data.get_filter(m_main_filter).adjust_left();
		}
		else if (menu_event.iptkey == IPT_UI_RIGHT)
		{
			changed = m_filter_data.get_filter(m_main_filter).adjust_right();
		}
		else if (menu_event.iptkey == IPT_UI_SELECT)
		{
			m_filter_data.get_filter(m_main_filter).show_ui(
					ui(),
					container(),
					[this] (machine_filter &filter)
					{
						if (machine_filter::CUSTOM == filter.get_type())
						{
							emu_file file(ui().options().ui_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
							if (!file.open(util::string_format("custom_%s_filter.ini", emulator_info::get_configname())))
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
		if (menu_event.iptkey == IPT_UI_SELECT)
			menu::stack_push<menu_directory>(ui(), container());
		break;
	case CUSTOM_MENU:
		if (menu_event.iptkey == IPT_UI_SELECT)
			menu::stack_push<menu_custom_ui>(ui(), container(), [this] () { reset(reset_options::REMEMBER_REF); });
		break;
	default:
		menu_simple_game_options::handle_item_event(menu_event);
		return;
	}

	if (changed)
		reset(reset_options::REMEMBER_REF);
}

} // namespace ui
