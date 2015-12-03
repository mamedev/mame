// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/tapectrl.h

    Tape control

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
	virtual void populate() override;
	virtual void handle() override;

private:
	static void get_time_string(std::string &dest, cassette_image_device *cassette, int *curpos, int *endpos);
};

#endif /* __UI_TAPECTRL_H__ */
