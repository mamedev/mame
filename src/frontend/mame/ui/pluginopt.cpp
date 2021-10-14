// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

    ui/pluginopt.cpp

    Internal menu for the plugin interface.

*********************************************************************/

#include "emu.h"
#include "pluginopt.h"

#include "ui/utils.h"

#include "luaengine.h"
#include "mame.h"


namespace ui {

void menu_plugin::handle()
{
	const event *menu_event = process(0);

	if (menu_event && menu_event->itemref)
	{
		if (menu_event->iptkey == IPT_UI_SELECT)
			menu::stack_push<menu_plugin_opt>(ui(), container(), (char *)menu_event->itemref);
	}
}

menu_plugin::menu_plugin(mame_ui_manager &mui, render_container &container) :
	menu(mui, container),
	m_plugins(mame_machine_manager::instance()->lua()->get_menu())
{
}

void menu_plugin::populate(float &customtop, float &custombottom)
{
	for (auto &curplugin : m_plugins)
		item_append(curplugin, 0, (void *)curplugin.c_str());
	item_append(menu_item_type::SEPARATOR);
}

void menu_plugin::show_menu(mame_ui_manager &mui, render_container &container, char *menu)
{
	// reset the menu stack
	menu::stack_reset(mui.machine());

	// add the plugin menu entry
	menu::stack_push<menu_plugin_opt>(mui, container, menu);

	// force the menus on
	mui.show_menu();

	// make sure MAME is paused
	mui.machine().pause();
}

menu_plugin::~menu_plugin()
{
}

menu_plugin_opt::menu_plugin_opt(mame_ui_manager &mui, render_container &container, char *menu) :
		ui::menu(mui, container),
		m_menu(menu)
{
}

void menu_plugin_opt::handle()
{
	const event *menu_event = process(0);

	if (menu_event && uintptr_t(menu_event->itemref))
	{
		std::string key;
		switch (menu_event->iptkey)
		{
			case IPT_UI_UP:
				key = "up";
				break;
			case IPT_UI_DOWN:
				key = "down";
				break;
			case IPT_UI_LEFT:
				key = "left";
				break;
			case IPT_UI_RIGHT:
				key = "right";
				break;
			case IPT_UI_SELECT:
				key = "select";
				break;
			case IPT_UI_DISPLAY_COMMENT:
				key = "comment";
				break;
			case IPT_UI_CLEAR:
				key = "clear";
				break;
			case IPT_UI_CANCEL:
				key = "cancel";
				break;
			case IPT_SPECIAL:
				key = std::to_string((u32)menu_event->unichar);
				break;
			default:
				return;
		}
		if (mame_machine_manager::instance()->lua()->menu_callback(m_menu, uintptr_t(menu_event->itemref), key))
			reset(reset_options::REMEMBER_REF);
		else if (menu_event->iptkey == IPT_UI_CANCEL)
			stack_pop();
	}
}

void menu_plugin_opt::populate(float &customtop, float &custombottom)
{
	std::vector<std::tuple<std::string, std::string, std::string>> menu_list;
	mame_machine_manager::instance()->lua()->menu_populate(m_menu, menu_list);
	uintptr_t i = 1;
	for(auto &item : menu_list)
	{
		const std::string &text = std::get<0>(item);
		const std::string &subtext = std::get<1>(item);
		const std::string &tflags = std::get<2>(item);

		uint32_t flags = 0;
		if(tflags == "off")
			flags = FLAG_DISABLE;
		else if(tflags == "l")
			flags = FLAG_LEFT_ARROW;
		else if(tflags == "r")
			flags = FLAG_RIGHT_ARROW;
		else if(tflags == "lr")
			flags = FLAG_RIGHT_ARROW | FLAG_LEFT_ARROW;

		if(text == "---")
		{
			item_append(menu_item_type::SEPARATOR);
			i++;
		}
		else
			item_append(text, subtext, flags, (void *)i++);
	}
	item_append(menu_item_type::SEPARATOR);
}

menu_plugin_opt::~menu_plugin_opt()
{
}

} // namespace ui
