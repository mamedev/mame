// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/cheatopt.h

    Internal menu for the cheat interface.

***************************************************************************/

#pragma once

#ifndef __UI_CHEATOPT_H__
#define __UI_CHEATOPT_H__

class ui_menu_cheat : public ui_menu {
public:
	ui_menu_cheat(running_machine &machine, render_container *container);
	virtual ~ui_menu_cheat();
	virtual void populate() override;
	virtual void handle() override;
};

#endif  /* __UI_CHEATOPT_H__ */
