// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#ifndef MAME_MACHINE_IE15_KBD_H
#define MAME_MACHINE_IE15_KBD_H

#pragma once

#include "machine/keyboard.h"

class ie15_keyboard_device : public device_t,  protected device_matrix_keyboard_interface<4U>
{
public:
	enum
	{
		IE_KB_ACK   = 1,

		IE_KB_RED   = 0x01,
		IE_KB_SDV   = 0x02,
		IE_KB_DUP   = 0x08,
		IE_KB_LIN   = 0x10,
		IE_KB_DK    = 0x20,
		IE_KB_PCH   = 0x40,
		IE_KB_NR    = 0x80,

		IE_KB_RED_BIT   = 0,
		IE_KB_SDV_BIT   = 1,
		IE_KB_DUP_BIT   = 3,
		IE_KB_LIN_BIT   = 4,
		IE_KB_DK_BIT    = 5,
		IE_KB_PCH_BIT   = 6,
		IE_KB_NR_BIT    = 7,
	};

	ie15_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto keyboard_cb() { return m_keyboard_cb.bind(); }
	auto sdv_cb() { return m_sdv_cb.bind(); }

	void set_ruslat(bool state) { m_ruslat = state; }

	DECLARE_INPUT_CHANGED_MEMBER(dip_changed);

protected:
	ie15_keyboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void key_make(uint8_t row, uint8_t column) override;

	required_ioport m_io_kbdc;

private:
	required_region_ptr<uint8_t> m_rom;
	devcb_write16 m_keyboard_cb;
	devcb_write_line m_sdv_cb;
	bool m_ruslat;
};

DECLARE_DEVICE_TYPE(IE15_KEYBOARD, ie15_keyboard_device)

#endif // MAME_MACHINE_IE15_KBD_H
