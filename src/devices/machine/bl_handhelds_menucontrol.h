// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_BL_HANDHELDS_MENUCONTROL_H
#define MAME_MACHINE_BL_HANDHELDS_MENUCONTROL_H

#pragma once

DECLARE_DEVICE_TYPE(BL_HANDHELDS_MENUCONTROL, bl_handhelds_menucontrol_device)

class bl_handhelds_menucontrol_device : public device_t
{
public:
	// construction/destruction
	bl_handhelds_menucontrol_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// the chip is the same between systems, but there's some logic not fully understood that is causing off-by-1 errors on some calcs
	void set_is_unsp_type_hack() { m_is_unsp_type_hack = true; }

	int status_r();
	int data_r();
	void clock_w(int state);
	void data_w(int state);
	void reset_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// config
	bool m_is_unsp_type_hack;

	// internal state
	uint16_t m_menupos;

	// command handling
	uint8_t m_clockstate;
	uint8_t m_datashifterpos;

	uint8_t m_responsebit;
	uint8_t m_response;

	uint8_t m_commandbit;
	uint8_t m_command;

	void handle_command();

	enum menustate : uint8_t
	{
	   MENU_READY_FOR_COMMAND = 0,

	   MENU_COMMAND_00_IN,
	   MENU_COMMAND_01_IN,
	   MENU_COMMAND_02_IN,
	   MENU_COMMAND_03_IN,
	   MENU_COMMAND_04_IN,
	   MENU_COMMAND_05_IN,
	};

	uint8_t m_menustate;
};

#endif // MAME_MACHINE_BL_HANDHELDS_MENUCONTROL_H
