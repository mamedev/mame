// license:BSD-3-Clause
// copyright-holders:Dankan1890
/***************************************************************************

    mewui/ctrlmenu.h

    Internal MEWUI user interface.

***************************************************************************/
#pragma once

#ifndef __MEWUI_CTRLMENU_H__
#define __MEWUI_CTRLMENU_H__

//-------------------------------------------------
//  class controller mapping menu
//-------------------------------------------------

class ui_menu_controller_mapping : public ui_menu
{
public:
	ui_menu_controller_mapping(running_machine &machine, render_container *container);
	virtual ~ui_menu_controller_mapping();
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	struct ctrl_option
	{
		int         status;
		const char  *description;
		const char  *option;
	};

	static const char *m_device_status[];
	static ctrl_option m_options[];
	int check_status(const char *status, const char *option);
};

#endif /* __MEWUI_CTRLMENU_H__ */
