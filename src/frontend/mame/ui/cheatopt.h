// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/cheatopt.h

    Internal menu for the cheat interface.

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_CHEATOPT_H
#define MAME_FRONTEND_UI_CHEATOPT_H

#include "ui/menu.h"

namespace ui {

class menu_cheat : public menu
{
public:
	menu_cheat(mame_ui_manager &mui, render_container *container);
	virtual ~menu_cheat() override;
	virtual void populate() override;
	virtual void handle() override;
};


class menu_autofire : public menu
{
public:
	menu_autofire(mame_ui_manager &mui, render_container *container);
	virtual ~menu_autofire() override;
	virtual void populate() override;
	virtual void handle() override;

private:
	float refresh;
	bool last_toggle;
};

} // namespace ui

#endif  /* MAME_FRONTEND_UI_CHEATOPT_H */
