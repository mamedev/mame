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
	menu_slot_devices(mame_ui_manager &mui, render_container &container);
	virtual ~menu_slot_devices() override;

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle() override;

	device_slot_option *get_current_option(device_slot_interface &slot) const;
	int get_current_index(device_slot_interface &slot) const;
	const char *get_next_slot(device_slot_interface &slot) const;
	const char *get_previous_slot(device_slot_interface &slot) const;
	void set_slot_device(device_slot_interface &slot, const char *val);
};

} // namespace ui

#endif  /* MAME_FRONTEND_UI_SLOTOPT_H */
