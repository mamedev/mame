/***************************************************************************

    ui/tapectrl.h

    MESS's tape control

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UI_TAPECTRL_H__
#define __UI_TAPECTRL_H__

#include "imagedev/cassette.h"

class ui_menu_mess_tape_control : public ui_menu {
public:
	ui_menu_mess_tape_control(running_machine &machine, render_container *container);
	virtual ~ui_menu_mess_tape_control();
	virtual void populate();
	virtual void handle();

private:
	int index;
	device_image_interface *device;
	int cassette_count();

	static astring &tapecontrol_gettime(astring &dest, cassette_image_device *cassette, int *curpos, int *endpos);
};

#endif /* __UI_TAPECTRL_H__ */
