// license:BSD-3-Clause
// copyright-holders:F.Ulivi
/***************************************************************************

    ui/info_pty.h

    Information screen on pseudo terminals

***************************************************************************/

#pragma once

#ifndef __UI_INFO_PTY_H__
#define __UI_INFO_PTY_H__

class ui_menu_pty_info : public ui_menu {
public:
	ui_menu_pty_info(running_machine &machine, render_container *container);
	virtual ~ui_menu_pty_info();
	virtual void populate();
	virtual void handle();
};

#endif // __UI_INFO_PTY_H__
