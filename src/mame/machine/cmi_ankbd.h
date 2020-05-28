// license:BSD-3-Clause
// copyright-holders:Phil Bennett
/***************************************************************************

    Fairlight Intelligent Alphanumeric Keyboard

***************************************************************************/

#ifndef MAME_MACHINE_CMI_ANKBD_H
#define MAME_MACHINE_CMI_ANKBD_H

#pragma once

#include "machine/6821pia.h"

class cmi_alphanumeric_keyboard_device : public device_t
{
public:
	cmi_alphanumeric_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto txd_handler() { return m_txd_handler.bind(); }
	auto rts_handler() { return m_rts_handler.bind(); }

	DECLARE_WRITE_LINE_MEMBER( rxd_w );
	DECLARE_WRITE_LINE_MEMBER( cts_w );

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_resolve_objects() override;
	virtual void device_start() override;

private:
	u8 col_r();
	DECLARE_WRITE_LINE_MEMBER( txd_w );
	DECLARE_WRITE_LINE_MEMBER( rts_w );

	void alphakeys_map(address_map &map);

	devcb_write_line m_txd_handler;
	devcb_write_line m_rts_handler;

	required_device<cpu_device> m_kbdcpu;
	required_device<pia6821_device> m_pia;

	required_ioport_array<8> m_row_ports;
};

DECLARE_DEVICE_TYPE(CMI_ALPHANUMERIC_KEYBOARD, cmi_alphanumeric_keyboard_device)

#endif // MAME_MACHINE_CMI_ANKBD_H
