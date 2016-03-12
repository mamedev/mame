// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/miscmenu.h

    Internal MAME menus for the user interface.

***************************************************************************/

#pragma once

#ifndef __UI_SLIDERS_H__
#define __UI_SLIDERS_H__

#include <map>

class ui_menu_sliders : public ui_menu {
public:
	ui_menu_sliders(running_machine &machine, render_container *container, bool menuless_mode = false);
	virtual ~ui_menu_sliders();
	virtual void populate() override;
	virtual void handle() override;

	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

	static UINT32 ui_handler(running_machine &machine, render_container *container, UINT32 state);

private:
	bool m_menuless_mode;
	bool m_hidden;
};


#endif  /* __UI_SLIDERS_H__ */
