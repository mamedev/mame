// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    ui/devopt.h

    Internal menu for the device configuration.

***************************************************************************/

#pragma once

#ifndef __UI_DEVOPT_H__
#define __UI_DEVOPT_H__

class ui_menu_device_config : public ui_menu {
public:
	ui_menu_device_config(running_machine &machine, render_container *container, device_slot_interface *slot, device_slot_option *option);
	virtual ~ui_menu_device_config();
	virtual void populate() override;
	virtual void handle() override;

private:
	device_slot_interface *m_owner;
	device_slot_option *m_option;
	bool m_mounted;
};


#endif  /* __UI_DEVOPT_H__ */
