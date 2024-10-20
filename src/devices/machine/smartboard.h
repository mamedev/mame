// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, hap
/*

    Tasc SmartBoard SB30

*/

#ifndef MAME_MACHINE_SMARTBOARD_H
#define MAME_MACHINE_SMARTBOARD_H

#pragma once

#include "machine/sensorboard.h"

class tasc_sb30_device : public device_t
{
public:
	tasc_sb30_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// configuration helpers
	auto data_out() { return m_data_out.bind(); } // data_r
	auto led_out() { return m_led_out.bind(); } // optional, outputs to sb30_ledy.x when not used

	// external read/write lines
	void data0_w(int state);
	void data1_w(int state);
	int data_r() { return m_output; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<sensorboard_device> m_board;
	output_finder<8, 8> m_out_leds;
	required_ioport m_conf;

	devcb_write_line m_data_out;
	devcb_write8 m_led_out;

	void update_output();
	bool piece_available(u8 id);
	void init_cb(u8 data);
	u8 spawn_cb(offs_t offset);

	// i/o lines
	int m_data0;
	int m_data1;
	int m_output;

	// internal use
	bool m_scan_pending;
	u32 m_pos;
	u32 m_squares[64]; // board state
};


DECLARE_DEVICE_TYPE(TASC_SB30, tasc_sb30_device)

#endif // MAME_MACHINE_SMARTBOARD_H
