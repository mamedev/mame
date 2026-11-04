// license:BSD-3-Clause
// copyright-holders:Tomás García-Merás Capote (ClawGrip)

#ifndef MAME_SEGA_M50DASS_H
#define MAME_SEGA_M50DASS_H

#pragma once

#include "315_5296.h"

#include "machine/pit8253.h"
#include "sound/ymopl.h"

class m50dass_device : public device_t, public device_mixer_interface
{
public:
	m50dass_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// player controls, only watched for activity
	auto p1_in_cb() { return m_p1_in_cb.bind(); }
	auto p2_in_cb() { return m_p2_in_cb.bind(); }

	// game board A seat signals, high to move the seat
	template <u32 N> void seat_w(int state);

	// game board A inputs, active low: d0 start 1, d1 start 2, d2 coin 1, d3 coin 2, d4 service, d5 test
	u8 board_inputs_r();

	void reset_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<pit8254_device> m_pit;
	required_device_array<sega_315_5296_device, 2> m_io;
	required_device<ym2413_device> m_ym2413;
	output_finder<2> m_digits;
	output_finder<2> m_start_lamps;
	output_finder<2> m_seat_motors;
	devcb_read8 m_p1_in_cb;
	devcb_read8 m_p2_in_cb;

	u8 m_seats;

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	u8 output_pins(int which, offs_t port, u8 data);
	void io0_portc_w(offs_t offset, u8 data);
	template <u32 N> void digit_w(offs_t offset, u8 data);
	void io1_portg_w(offs_t offset, u8 data);
};

DECLARE_DEVICE_TYPE(MEGALO50_DASS, m50dass_device)

#endif // MAME_SEGA_M50DASS_H
