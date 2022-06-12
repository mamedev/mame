// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ui/inputopts.h

    Input options submenu.

***************************************************************************/

#ifndef MAME_FRONTEND_UI_INPUTOPTS_H
#define MAME_FRONTEND_UI_INPUTOPTS_H

#pragma once

#include "ui/menu.h"


namespace ui {

class menu_input_options : public menu
{
public:
	menu_input_options(mame_ui_manager &mui, render_container &container);
	virtual ~menu_input_options();

protected:
	virtual void menu_activated() override;

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle(event const *ev) override;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_INPUTOPTS_H
