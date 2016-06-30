// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

    ui/pluginopt.cpp

    Internal menu for the plugin interface.

*********************************************************************/

#include "emu.h"

#include "ui/pluginopt.h"

#include "mame.h"
#include "luaengine.h"


namespace ui {
void menu_plugin::handle()
{
	const event *menu_event = process(0);

	if (menu_event != nullptr && menu_event->itemref != nullptr)
	{
		if (menu_event->iptkey == IPT_UI_SELECT)
			menu::stack_push<menu_plugin_opt>(ui(), container, (char *)menu_event->itemref);
	}
}

menu_plugin::menu_plugin(mame_ui_manager &mui, render_container *container) :
		menu(mui, container),
		m_plugins(mame_machine_manager::instance()->lua()->get_menu())
{
}

void menu_plugin::populate()
{
	for (auto &curplugin : m_plugins)
		item_append(curplugin, "", 0, (void *)curplugin.c_str());
	item_append(menu_item_type::SEPARATOR);
}

menu_plugin::~menu_plugin()
{
}

menu_plugin_opt::menu_plugin_opt(mame_ui_manager &mui, render_container *container, char *menu) :
		ui::menu(mui, container),
		m_menu(menu)
{
}

void menu_plugin_opt::handle()
{
	const event *menu_event = process(0);

	if (menu_event != nullptr && (FPTR)menu_event->itemref)
	{
		std::string key;
		switch(menu_event->iptkey)
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
			default:
				return;
		}
		if(mame_machine_manager::instance()->lua()->menu_callback(m_menu, (FPTR)menu_event->itemref, key))
			reset(reset_options::REMEMBER_REF);
	}
}

void menu_plugin_opt::populate()
{
	std::vector<lua_engine::menu_item> menu_list;
	mame_machine_manager::instance()->lua()->menu_populate(m_menu, menu_list);
	FPTR i = 1;
	for(auto &item : menu_list)
	{
		UINT32 flags = 0;
		if(item.flags == "off")
			flags = FLAG_DISABLE;
		else if(item.flags == "l")
			flags = FLAG_LEFT_ARROW;
		else if(item.flags == "r")
			flags = FLAG_RIGHT_ARROW;
		else if(item.flags == "lr")
			flags = FLAG_RIGHT_ARROW | FLAG_LEFT_ARROW;

		item_append(item.text.c_str(), item.subtext.c_str(), flags, (void *)i++);
	}
	item_append(menu_item_type::SEPARATOR);
}

menu_plugin_opt::~menu_plugin_opt()
{
}

} // namespace ui
