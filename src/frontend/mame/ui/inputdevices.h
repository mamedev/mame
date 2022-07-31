// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ui/inputdevices.h

    Input devices menu.

***************************************************************************/

#ifndef MAME_FRONTEND_UI_INPUTDEVICES_H
#define MAME_FRONTEND_UI_INPUTDEVICES_H

#pragma once

#include "ui/menu.h"


namespace ui {

class menu_input_devices : public menu
{
public:
	menu_input_devices(mame_ui_manager &mui, render_container &container);
	virtual ~menu_input_devices();

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle(event const *ev) override;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_INPUTDEVICES_H
