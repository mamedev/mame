// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods, Carl
/***************************************************************************

    ui/pluginopt.h

    Internal menu for the plugin interface.

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_PLUGINOPT_H
#define MAME_FRONTEND_UI_PLUGINOPT_H

#include "ui/ui.h"
#include "ui/menu.h"

#include <string>
#include <vector>


namespace ui {
class menu_plugin : public menu
{
public:
	menu_plugin(mame_ui_manager &mui, render_container &container);
	virtual ~menu_plugin();

private:
	virtual void populate() override;
	virtual void handle() override;

	std::vector<std::string> &m_plugins;
};

class menu_plugin_opt : public menu
{
public:
	menu_plugin_opt(mame_ui_manager &mui, render_container &container, char *menu);
	virtual ~menu_plugin_opt();

private:
	virtual void populate() override;
	virtual void handle() override;

	std::string m_menu;
};

} // namespace ui

#endif  /* MAME_FRONTEND_UI_PLUGINOPT_H */
