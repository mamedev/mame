/***************************************************************************

    ui/tapectrl.h

    Tape control

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UI_TAPECTRL_H__
#define __UI_TAPECTRL_H__

#include "imagedev/cassette.h"
#include "ui/devctrl.h"

class ui_menu_tape_control : public ui_menu_device_control<cassette_image_device> {
public:
	ui_menu_tape_control(running_machine &machine, render_container *container, cassette_image_device *device);
	virtual ~ui_menu_tape_control();
	virtual void populate();
	virtual void handle();

private:
	static void get_time_string(astring &dest, cassette_image_device *cassette, int *curpos, int *endpos);
};

#endif /* __UI_TAPECTRL_H__ */
