// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_NINTENDO_VT_MENU_PROTECTION_H
#define MAME_NINTENDO_VT_MENU_PROTECTION_H

#pragma once

class vt_menu_protection_device :  public device_t
{
public:
	vt_menu_protection_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read();
	void write_clock(int state);
	void write_data(int state);
	void write_enable(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_memory_region m_extrarom;

	u8 m_command;
	u8 m_commandbits;
	u8 m_protectionstate;
	u8 m_protlatch;
	u16 m_protreadposition;
	bool m_in_data;
	bool m_in_enable;
	bool m_in_clock;
};


DECLARE_DEVICE_TYPE(VT_MENU_PROTECTION, vt_menu_protection_device)

#endif // MAME_NINTENDO_VT_MENU_PROTECTION_H
