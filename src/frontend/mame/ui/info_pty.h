// license:BSD-3-Clause
// copyright-holders:F.Ulivi
/***************************************************************************

    ui/info_pty.h

    Information screen on pseudo terminals

***************************************************************************/

#ifndef MAME_FRONTEND_UI_INFO_PTY_H
#define MAME_FRONTEND_UI_INFO_PTY_H

#pragma once

#include "ui/menu.h"

namespace ui {

class menu_pty_info : public menu
{
public:
	menu_pty_info(mame_ui_manager &mui, render_container &container);
	virtual ~menu_pty_info() override;

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle(event const *ev) override;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_INFO_PTY_H
