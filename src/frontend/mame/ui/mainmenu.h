// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/mainmenu.h

    Internal MAME menus for the user interface.

***************************************************************************/

#ifndef MAME_FRONTEND_UI_MAINMENU_H
#define MAME_FRONTEND_UI_MAINMENU_H

#pragma once

#include "ui/menu.h"


namespace ui {

class menu_main : public menu
{
public:
	menu_main(mame_ui_manager &mui, render_container &container);
	virtual ~menu_main();

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle() override;
};

} // namespace ui

#endif  /* MAME_FRONTEND_UI_MAINMENU_H */
