// license:BSD-3-Clause
// copyright-holders:tim lindner
/**********************************************************************

    Radio Shack TRS-80 Color Graphics Printer (CGP-220, 26-1268)

    Four colour (black, cyan, magenta, yellow) ink jet printer with
    both a Radio Shack serial and a Centronics parallel interface. The
    rs232 and centronics slot devices both wrap this one.

**********************************************************************/

#ifndef MAME_MACHINE_CGP220_H
#define MAME_MACHINE_CGP220_H

#pragma once

#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/bitmap_printer.h"


class cgp220_device : public device_t
{
public:
	cgp220_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// interface select switch: serial or parallel
	void set_serial_interface(bool serial) { m_serial = serial; }

	auto busy_handler() { return m_busy_cb.bind(); }
	auto ack_handler() { return m_ack_cb.bind(); }
	auto perror_handler() { return m_perror_cb.bind(); }
	auto fault_handler() { return m_fault_cb.bind(); }
	auto select_handler() { return m_select_cb.bind(); }

	void rxd_w(int state);
	void strobe_w(int state);
	void data_w(u8 data) { m_data = data; }
	template <unsigned Bit> void data_bit_w(int state) { m_data = (m_data & ~(1U << Bit)) | ((state ? 1U : 0U) << Bit); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	u8 pia_pa_r();
	void pia_pa_w(u8 data);
	void pia_pb_w(u8 data);
	u8 data_r();
	void control_w(u8 data);
	u8 status_r();
	void head_fire_w(u8 data);

	TIMER_CALLBACK_MEMBER(carriage_tick);
	int sensor(int pos) const;
	void step_paper(u8 phases);
	void update_head();
	void update_outputs();

	required_device<mc6809_device> m_maincpu;
	required_device<pia6821_device> m_pia;
	required_device<ptm6840_device> m_ptm;
	required_device<bitmap_printer_device> m_bitmap_printer;
	required_ioport m_buttons;
	required_ioport m_switches;
	output_finder<> m_ready_led;
	output_finder<> m_alert_led;

	devcb_write_line m_busy_cb;
	devcb_write_line m_ack_cb;
	devcb_write_line m_perror_cb;
	devcb_write_line m_fault_cb;
	devcb_write_line m_select_cb;

	emu_timer *m_carriage_timer;

	bool m_serial;
	int m_rxd;
	int m_strobe;
	u8 m_data;
	u8 m_latch;
	u8 m_control;
	bool m_busy_ff;
	u8 m_heads;
	u8 m_motor;
	u8 m_paper_phase;
	int m_paper_steps;
	double m_cr_pos;
	double m_cr_vel;
	int m_cr_cell;
};

DECLARE_DEVICE_TYPE(CGP220, cgp220_device)

#endif // MAME_MACHINE_CGP220_H
