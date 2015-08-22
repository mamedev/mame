// license:BSD-3-Clause
// copyright-holders:Dankan1890
/***************************************************************************

    mewui/miscmenu.h

    MEWUI miscellaneous options menu.

***************************************************************************/
#pragma once

#ifndef __MEWUI_MISCMENU_H__
#define __MEWUI_MISCMENU_H__

struct misc_option
{
	bool        status;
	const char  *description;
	const char  *option;
};

//-------------------------------------------------
//  class miscellaneous options menu
//-------------------------------------------------
class ui_menu_misc_options : public ui_menu
{
public:
	ui_menu_misc_options(running_machine &machine, render_container *container);
	virtual ~ui_menu_misc_options();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:
	static misc_option m_options[];
};

#endif /* __MEWUI_MISCMENU_H__ */
