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
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:
	enum
	{
		LIGHTGUN_DEVICE_ASSIGN = 1,
		TRACKBALL_DEVICE_ASSIGN,
		ADSTICK_DEVICE_ASSIGN,
		PADDLE_DEVICE_ASSIGN,
		DIAL_DEVICE_ASSIGN,
		POSITIONAL_DEVICE_ASSIGN,
		MOUSE_DEVICE_ASSIGN,
		LAST_DEVICE_ASSIGN
	};

	static const char *device_status[];
	int check_status(const char *status);
	int m_options[LAST_DEVICE_ASSIGN];
};

#endif /* __MEWUI_CTRLMENU_H__ */
