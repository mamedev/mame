/***************************************************************************

    ui/bbcontrl.h

    MESS's "bit banger" control

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UI_BBCONTRL_H__
#define __UI_BBCONTRL_H__

class ui_menu_mess_bitbanger_control : public ui_menu {
public:
	ui_menu_mess_bitbanger_control(running_machine &machine, render_container *container);
	virtual ~ui_menu_mess_bitbanger_control();
	virtual void populate();
	virtual void handle();

private:
	int index;
	device_image_interface *device;
	int bitbanger_count();
};

#endif // __UI_BBCONTRL_H__
