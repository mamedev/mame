// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods, Carl
/***************************************************************************

    ui/pluginopt.h

    Internal menu for the plugin interface.

***************************************************************************/
#ifndef MAME_FRONTEND_UI_PLUGINOPT_H
#define MAME_FRONTEND_UI_PLUGINOPT_H

#pragma once

#include "ui/menu.h"
#include "ui/ui.h"

#include <string>
#include <string_view>
#include <vector>


namespace ui {

class menu_plugin : public menu
{
public:
	menu_plugin(mame_ui_manager &mui, render_container &container);

	static void show_menu(mame_ui_manager &mui, render_container &container, char *menu);

	virtual ~menu_plugin();

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle(event const *ev) override;

	std::vector<std::string> &m_plugins;
};

class menu_plugin_opt : public menu
{
public:
	menu_plugin_opt(mame_ui_manager &mui, render_container &container, std::string_view menu);
	virtual ~menu_plugin_opt();

protected:
	virtual bool custom_ui_cancel() override { return true; }

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle(event const *ev) override;

	std::string const m_menu;
	bool m_need_idle;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_PLUGINOPT_H
