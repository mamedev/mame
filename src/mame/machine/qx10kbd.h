// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_MACHINE_QX10KBD_H
#define MAME_MACHINE_QX10KBD_H

#pragma once

#include "bus/rs232/rs232.h"
#include "machine/keyboard.h"


class qx10_keyboard_device
	: public device_t
	, public device_rs232_port_interface
{
public:
	qx10_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ioport_constructor device_input_ports() const override;
	virtual DECLARE_WRITE_LINE_MEMBER(input_txd) override;
	DECLARE_WRITE8_MEMBER(mcu_p1_w);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void device_start() override;

private:
	required_ioport_array<16> m_rows;
	required_device<cpu_device> m_mcu;
	emu_timer *m_bit_timer;
	u8 m_rxd;
	int m_row, m_clk_state;
};

DECLARE_DEVICE_TYPE(QX10_KEYBOARD, qx10_keyboard_device)

#endif // MAME_MACHINE_QX10KBD_H
