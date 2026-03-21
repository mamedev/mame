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

bool menu_plugin::handle(event const *ev)
{
	if (ev && ev->itemref)
	{
		if (ev->iptkey == IPT_UI_SELECT)
			menu::stack_push<menu_plugin_opt>(ui(), container(), (char *)ev->itemref, false);
	}
	return false;
}

menu_plugin::menu_plugin(mame_ui_manager &mui, render_container &container) :
	menu(mui, container),
	m_plugins(mame_machine_manager::instance()->lua()->get_menu())
{
	set_heading(_("menu-pluginopts", "Plugin Options"));
}

void menu_plugin::populate()
{
	for (auto &curplugin : m_plugins)
		item_append(curplugin, 0, (void *)curplugin.c_str());
	item_append(menu_item_type::SEPARATOR);
}

void menu_plugin::show_menu(mame_ui_manager &mui, render_container &container, std::string_view menu)
{
	// add the plugin menu entry
	menu::stack_push<menu_plugin_opt>(mui, container, menu, true);

	// force the menus on
	mui.show_menu();
}

menu_plugin::~menu_plugin()
{
}

menu_plugin_opt::menu_plugin_opt(mame_ui_manager &mui, render_container &container, std::string_view menu, bool one_shot) :
	ui::menu(mui, container),
	m_menu(menu),
	m_need_idle(false)
{
	set_needs_prev_menu_item(false);
	set_one_shot(one_shot);
}

bool menu_plugin_opt::handle(event const *ev)
{
	void *const itemref = ev ? ev->itemref : get_selection_ref();
	auto const itemrefno = uintptr_t(itemref);
	std::string key;
	if (ev)
	{
		switch (ev->iptkey)
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
		case IPT_UI_PREV_GROUP:
			key = "prevgroup";
			break;
		case IPT_UI_NEXT_GROUP:
			key = "nextgroup";
			break;
		case IPT_UI_SELECT:
			if (!itemrefno)
			{
				stack_pop();
				return false;
			}
			key = "select";
			break;
		case IPT_UI_DISPLAY_COMMENT:
			key = "comment";
			break;
		case IPT_UI_CLEAR:
			key = "clear";
			break;
		case IPT_UI_BACK:
			key = "back";
			break;
		case IPT_UI_CANCEL:
			key = "cancel";
			break;
		case IPT_SPECIAL:
			key = std::to_string((u32)ev->unichar);
			break;
		default:
			break;
		}
	}

	if (key.empty() && !m_need_idle)
		return false;

	auto const result = mame_machine_manager::instance()->lua()->menu_callback(m_menu, itemrefno, key);
	if (result.second)
	{
		auto const selno = uintptr_t(*result.second);
		if (selno >= 1)
			set_selection(reinterpret_cast<void *>(selno));
	}
	if (result.first)
		reset(reset_options::REMEMBER_REF);
	else if (ev && (ev->iptkey == IPT_UI_BACK))
		stack_pop();

	return result.second && !result.first;
}

void menu_plugin_opt::populate()
{
	std::vector<std::tuple<std::string, std::string, std::string>> menu_list;
	std::string flags;
	auto const sel = mame_machine_manager::instance()->lua()->menu_populate(m_menu, menu_list, flags);

	uintptr_t i = 1;
	for (auto &item : menu_list)
	{
		std::string &text = std::get<0>(item);
		std::string &subtext = std::get<1>(item);
		std::string_view tflags = std::get<2>(item);

		uint32_t item_flags_or = uint32_t(0);
		uint32_t item_flags_and = ~uint32_t(0);
		auto flag_start = tflags.find_first_not_of(' ');
		while (std::string_view::npos != flag_start)
		{
			tflags.remove_prefix(flag_start);
			auto const flag_end = tflags.find(' ');
			auto const flag = tflags.substr(0, flag_end);
			tflags.remove_prefix(flag.length());
			flag_start = tflags.find_first_not_of(' ');

			if (flag == "off")
				item_flags_or |= FLAG_DISABLE;
			else if (flag == "on")
				item_flags_and &= ~FLAG_DISABLE;
			else if (flag == "l")
				item_flags_or |= FLAG_LEFT_ARROW;
			else if (flag == "r")
				item_flags_or |= FLAG_RIGHT_ARROW;
			else if (flag == "lr")
				item_flags_or |= FLAG_RIGHT_ARROW | FLAG_LEFT_ARROW;
			else if (flag == "invert")
				item_flags_or |= FLAG_INVERT;
			else if (flag == "heading")
				item_flags_or |= FLAG_DISABLE | FLAG_UI_HEADING;
			else
				osd_printf_info("menu_plugin_opt: unknown flag '%s' for item %d (%s)\n", flag, i, text);
		}

		if (text == "---")
			item_append(menu_item_type::SEPARATOR);
		else
			item_append(std::move(text), std::move(subtext), item_flags_or & item_flags_and, reinterpret_cast<void *>(i));
		++i;
	}
	item_append(menu_item_type::SEPARATOR);
	item_append(
			is_one_shot() ? _("menu-pluginopts", "Close Menu") : _("menu-pluginopts", "Return to Plugin Options Menu"),
			0,
			reinterpret_cast<void *>(uintptr_t(0)));

	if (sel)
	{
		auto const selno = uintptr_t(*sel);
		if ((sel >= 1U) && (sel < i))
			set_selection(reinterpret_cast<void *>(selno));
	}

	uint32_t process_flags = 0U;
	m_need_idle = false;
	if (!flags.empty())
	{
		std::string_view mflags = flags;
		auto flag_start = mflags.find_first_not_of(' ');
		while (std::string_view::npos != flag_start)
		{
			mflags.remove_prefix(flag_start);
			auto const flag_end = mflags.find(' ');
			auto const flag = mflags.substr(0, flag_end);
			mflags.remove_prefix(flag.length());
			flag_start = mflags.find_first_not_of(' ');

			if (flag == "nokeys")
				process_flags |= PROCESS_NOKEYS;
			else if (flag == "lralways")
				process_flags |= PROCESS_LR_ALWAYS;
			else if (flag == "lrrepeat")
				process_flags |= PROCESS_LR_REPEAT;
			else if (flag == "customnav")
				process_flags |= PROCESS_CUSTOM_NAV;
			else if (flag == "ignorepause")
				process_flags |= PROCESS_IGNOREPAUSE;
			else if (flag == "idle")
				m_need_idle = true;
			else
				osd_printf_info("menu_plugin_opt: unknown processing flag '%s'\n", flag);
		}
		if (process_flags & PROCESS_NOKEYS)
			m_need_idle = true;
	}
	set_process_flags(process_flags);
}

menu_plugin_opt::~menu_plugin_opt()
{
}

} // namespace ui
