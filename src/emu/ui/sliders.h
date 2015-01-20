/***************************************************************************

    ui/miscmenu.h

    Internal MAME menus for the user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UI_SLIDERS_H__
#define __UI_SLIDERS_H__

class ui_menu_sliders : public ui_menu {
public:
	ui_menu_sliders(running_machine &machine, render_container *container, bool menuless_mode = false);
	virtual ~ui_menu_sliders();
	virtual void populate();
	virtual void handle();

	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

	static UINT32 ui_handler(running_machine &machine, render_container *container, UINT32 state);

private:
	bool menuless_mode, hidden;
};


#endif  /* __UI_SLIDERS_H__ */
