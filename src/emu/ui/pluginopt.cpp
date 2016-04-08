// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

    ui/pluginopt.cpp

    Internal menu for the plugin interface.

*********************************************************************/

#include "emu.h"
#include "luaengine.h"

#include "ui/pluginopt.h"

void ui_menu_plugin::handle()
{
	const ui_menu_event *menu_event = process(0);

	if (menu_event != nullptr && menu_event->itemref != nullptr)
	{
		if (menu_event->iptkey == IPT_UI_SELECT)
			ui_menu::stack_push(global_alloc_clear<ui_menu_plugin_opt>(machine(), container, (char *)menu_event->itemref));
	}
}

ui_menu_plugin::ui_menu_plugin(running_machine &machine, render_container *container) :
		ui_menu(machine, container),
		m_plugins(machine.manager().lua()->get_menu())
{
}

void ui_menu_plugin::populate()
{
	for (auto &curplugin : m_plugins)
		item_append(curplugin.c_str(), 0, 0, (void *)curplugin.c_str());
	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
}

ui_menu_plugin::~ui_menu_plugin()
{
}

ui_menu_plugin_opt::ui_menu_plugin_opt(running_machine &machine, render_container *container, char *menu) :
		ui_menu(machine, container),
		m_menu(menu)
{
}

void ui_menu_plugin_opt::handle()
{
	const ui_menu_event *menu_event = process(0);

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
		if(machine().manager().lua()->menu_callback(m_menu, (FPTR)menu_event->itemref, key))
			reset(UI_MENU_RESET_REMEMBER_REF);
	}
}

void ui_menu_plugin_opt::populate()
{
	std::vector<lua_engine::menu_item> &menu_list = machine().manager().lua()->menu_populate(m_menu);
	FPTR i = 1;
	for(auto &item : menu_list)
		item_append(item.text.c_str(), item.subtext.c_str(), item.flags, (void *)i++);
	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
	global_free(&menu_list);
}

ui_menu_plugin_opt::~ui_menu_plugin_opt()
{
}
