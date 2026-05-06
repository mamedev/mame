// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_NINTENDO_VT_MENU_PROTECTION_LXCAP_H
#define MAME_NINTENDO_VT_MENU_PROTECTION_LXCAP_H

#pragma once

DECLARE_DEVICE_TYPE(VT_MENU_PROTECTION_LXCAP, vt_menu_protection_lxcap_device)


class vt_menu_protection_lxcap_device :  public device_t
{
public:
	vt_menu_protection_lxcap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read();
	void write_clock(bool state);
	void write_data(bool state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	bool m_clock;
	bool m_data;

	u8 m_bitcount;
	u32 m_command;
	u8 m_phase;
	u8 m_retdat;
	u8 m_outlatch;
};

#endif // MAME_NINTENDO_VT_MENU_PROTECTION_LXCAP_H
