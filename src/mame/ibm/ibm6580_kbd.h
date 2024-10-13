// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#ifndef MAME_IBM_IBM6580_KBD_H
#define MAME_IBM_IBM6580_KBD_H

#pragma once

#include "cpu/mcs48/mcs48.h"

class dw_keyboard_device :  public device_t
{
public:
	dw_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_data_handler() { return m_out_data.bind(); }
	auto out_clock_handler() { return m_out_clock.bind(); }
	auto out_strobe_handler() { return m_out_strobe.bind(); }

	int memory_record_r() { return m_mr->read(); }

	void reset_w(int state);
	void ack_w(int state);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

private:
	uint8_t m_dip, m_bus, m_p2;
	int m_drive, m_sense, m_reset;
	int m_keylatch, m_ack;

	required_ioport_array<12> m_kbd;
	required_ioport m_mr;
	devcb_write_line m_out_data;
	devcb_write_line m_out_clock;
	devcb_write_line m_out_strobe;
	required_device<i8049_device> m_mcu;

	void bus_w(uint8_t data);
	void p1_w(uint8_t data);
	void p2_w(uint8_t data);
};

DECLARE_DEVICE_TYPE(DW_KEYBOARD, dw_keyboard_device)

#endif // MAME_IBM_IBM6580_KBD_H
