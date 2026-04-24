// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Nokia MikroMikko 2 keyboard emulation

*********************************************************************/

#ifndef MAME_BUS_MM2_MM2KB_H
#define MAME_BUS_MM2_MM2KB_H

#pragma once

#include "diserial.h"
#include "machine/keyboard.h"

DECLARE_DEVICE_TYPE(NOKIA_MM2_KBD, mm2_keyboard_device)

class mm2_keyboard_device :  public device_t,
							 public device_buffered_serial_interface<16U>,
							 protected device_matrix_keyboard_interface<11U>
{
public:
	// construction/destruction
	mm2_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto txd_handler() { return m_write_txd.bind(); }

	void rxd_w(int state) { device_buffered_serial_interface::rx_w(state); }

protected:
	mm2_keyboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void tra_callback() override { m_write_txd(transmit_register_get_data_bit()); }

	void key_make(u8 row, u8 column) override { transmit_byte((row << 4) + column); }
	void key_break(u8 row, u8 column) override { if (row > 7) transmit_byte(0x40 | ((row << 4) + column)); }

private:
	virtual void received_byte(uint8_t byte) override {};

	devcb_write_line m_write_txd;
};

#endif // MAME_BUS_MM2_MM2KB_H
