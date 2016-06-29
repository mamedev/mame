// license:BSD-3-Clause
// copyright-holders:F.Ulivi
/***************************************************************************

    ui/info_pty.h

    Information screen on pseudo terminals

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_INFO_PTY_H
#define MAME_FRONTEND_UI_INFO_PTY_H

#include "ui/menu.h"

namespace ui {
class menu_pty_info : public menu
{
public:
	menu_pty_info(mame_ui_manager &mui, render_container *container);
	virtual ~menu_pty_info() override;
	virtual void populate() override;
	virtual void handle() override;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_INFO_PTY_H
