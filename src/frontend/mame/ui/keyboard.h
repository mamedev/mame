// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ui/keyboard.h

    Keyboard mode menu.

***************************************************************************/
#ifndef MAME_FRONTEND_UI_KEYBOARD_H
#define MAME_FRONTEND_UI_KEYBOARD_H

#pragma once

#include "ui/menu.h"


namespace ui {

class menu_keyboard_mode : public menu
{
public:
	menu_keyboard_mode(mame_ui_manager &mui, render_container &container);
	virtual ~menu_keyboard_mode();

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle(event const *ev) override;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_KEYBOARD_H
