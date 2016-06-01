// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/slotopt.h

    Internal menu for the slot options.

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_SLOTOPT_H
#define MAME_FRONTEND_UI_SLOTOPT_H

#include "ui/menu.h"


namespace ui {

class menu_slot_devices : public menu
{
public:
	menu_slot_devices(mame_ui_manager &mui, render_container *container);
	virtual ~menu_slot_devices() override;
	virtual void populate() override;
	virtual void handle() override;

private:
	device_slot_option *slot_get_current_option(device_slot_interface &slot);
	int slot_get_current_index(device_slot_interface &slot);
	int slot_get_length(device_slot_interface &slot);
	const char *slot_get_next(device_slot_interface &slot);
	const char *slot_get_prev(device_slot_interface &slot);
	const char *slot_get_option(device_slot_interface &slot, int index);
	void set_slot_device(device_slot_interface &slot, const char *val);
};

} // namespace ui

#endif  /* MAME_FRONTEND_UI_SLOTOPT_H */
