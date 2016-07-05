// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    ui/devopt.h

    Internal menu for the device configuration.

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_DEVOPT_H
#define MAME_FRONTEND_UI_DEVOPT_H

#include "ui/menu.h"

namespace ui {
class menu_device_config : public menu
{
public:
	menu_device_config(mame_ui_manager &mui, render_container *container, device_slot_interface *slot, device_slot_option *option);
	virtual ~menu_device_config() override;
	virtual void populate() override;
	virtual void handle() override;

private:
	device_slot_interface *m_owner;
	device_slot_option *m_option;
	bool m_mounted;
};

} // namespace ui

#endif  /* MAME_FRONTEND_UI_DEVOPT_H */
