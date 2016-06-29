// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/tapectrl.h

    Tape control

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_TAPECTRL_H
#define MAME_FRONTEND_UI_TAPECTRL_H

#include "imagedev/cassette.h"
#include "ui/devctrl.h"

namespace ui {
class menu_tape_control : public menu_device_control<cassette_image_device>
{
public:
	menu_tape_control(mame_ui_manager &mui, render_container *container, cassette_image_device *device);
	virtual ~menu_tape_control() override;
	virtual void populate() override;
	virtual void handle() override;

private:
	static void get_time_string(std::string &dest, cassette_image_device *cassette, int *curpos, int *endpos);
};

} // namespace ui

#endif /* MAME_FRONTEND_UI_TAPECTRL_H */
