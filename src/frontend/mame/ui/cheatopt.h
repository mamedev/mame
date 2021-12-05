// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/cheatopt.h

    Internal menu for the cheat interface.

***************************************************************************/
#ifndef MAME_FRONTEND_UI_CHEATOPT_H
#define MAME_FRONTEND_UI_CHEATOPT_H

#pragma once

#include "ui/menu.h"


namespace ui {

class menu_cheat : public menu
{
public:
	menu_cheat(mame_ui_manager &mui, render_container &container);
	virtual ~menu_cheat() override;

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle(event const *ev) override;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_CHEATOPT_H
