// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_NINTENDO_VT_MENU_PROTECTION_H
#define MAME_NINTENDO_VT_MENU_PROTECTION_H

#pragma once

DECLARE_DEVICE_TYPE(VT_MENU_PROTECTION, vt_menu_protection_device)


class vt_menu_protection_device :  public device_t
{
public:
	vt_menu_protection_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read();
	void write_clock(bool state);
	void write_data(bool state);
	void write_enable(bool state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 m_command;
	u8 m_commandbits;
	u8 m_protectionstate;
	u8 m_protlatch;
	u16 m_protreadposition;
	bool m_data;
	bool m_enable;

	required_memory_region m_extrarom;
};

#endif // MAME_NINTENDO_VT_MENU_PROTECTION_H
