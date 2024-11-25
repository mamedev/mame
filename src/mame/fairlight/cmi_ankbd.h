// license:BSD-3-Clause
// copyright-holders:Phil Bennett
/***************************************************************************

    Fairlight Intelligent Alphanumeric Keyboard

***************************************************************************/

#ifndef MAME_FAIRLIGHT_CMI_ANKBD_H
#define MAME_FAIRLIGHT_CMI_ANKBD_H

#pragma once

#include "machine/6821pia.h"

class cmi_alphanumeric_keyboard_device : public device_t
{
public:
	cmi_alphanumeric_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto txd_handler() { return m_txd_handler.bind(); }
	auto rts_handler() { return m_rts_handler.bind(); }

	void rxd_w(int state);
	void cts_w(int state);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	u8 col_r();
	void txd_w(int state);
	void rts_w(int state);

	void alphakeys_map(address_map &map) ATTR_COLD;

	devcb_write_line m_txd_handler;
	devcb_write_line m_rts_handler;

	required_device<cpu_device> m_kbdcpu;
	required_device<pia6821_device> m_pia;

	required_ioport_array<8> m_row_ports;
};

DECLARE_DEVICE_TYPE(CMI_ALPHANUMERIC_KEYBOARD, cmi_alphanumeric_keyboard_device)

#endif // MAME_FAIRLIGHT_CMI_ANKBD_H
