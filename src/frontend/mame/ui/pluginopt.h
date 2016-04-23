// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods, Carl
/***************************************************************************

    ui/pluginopt.h

    Internal menu for the plugin interface.

***************************************************************************/

#pragma once

#ifndef __UI_PLUGINOPT_H__
#define __UI_PLUGINOPT_H__

#include "ui/ui.h"
#include "ui/menu.h"

class ui_menu_plugin : public ui_menu {
public:
	ui_menu_plugin(running_machine &machine, render_container *container);
	virtual ~ui_menu_plugin();
	virtual void populate() override;
	virtual void handle() override;
private:
	std::vector<std::string> &m_plugins;
};

class ui_menu_plugin_opt : public ui_menu {
public:
	ui_menu_plugin_opt(running_machine &machine, render_container *container, char *menu);
	virtual ~ui_menu_plugin_opt();
	virtual void populate() override;
	virtual void handle() override;
private:
	std::string m_menu;
};
#endif  /* __UI_PLUGINOPT_H__ */
