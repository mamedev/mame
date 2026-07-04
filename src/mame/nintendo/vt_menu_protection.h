// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_NINTENDO_VT_MENU_PROTECTION_H
#define MAME_NINTENDO_VT_MENU_PROTECTION_H

#pragma once

class vt_menu_protection_device :  public device_t
{
public:
	vt_menu_protection_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint8_t read();
	void write_clock(int state);
	void write_data(int state);
	void write_enable(int state);
	void set_read_start_byte(u8 byte) { m_read_start_byte = byte; }
	void enable_36pcase_late_protocol() { m_36pcase_late_protocol = true; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 compute_36pcase_late_response(u8 data);

	required_memory_region m_extrarom;

	u8 m_command;
	u8 m_commandbits;
	u8 m_protectionstate;
	u8 m_protlatch;
	u8 m_read_start_byte = 0;
	u16 m_protreadposition;
	u8 m_36pcase_late_response;
	u8 m_36pcase_late_response_pos;
	bool m_36pcase_late_protocol = false;
	bool m_in_data;
	bool m_in_enable;
	bool m_in_clock;
};


DECLARE_DEVICE_TYPE(VT_MENU_PROTECTION, vt_menu_protection_device)

#endif // MAME_NINTENDO_VT_MENU_PROTECTION_H
