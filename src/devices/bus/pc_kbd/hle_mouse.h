// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_BUS_HLE_PS2_MOUSE_H
#define MAME_BUS_HLE_PS2_MOUSE_H

#pragma once

#include "pc_kbdc.h"

class hle_ps2_mouse_device
	: public device_t
	, public device_pc_kbd_interface
{
public:
	hle_ps2_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_pc_kbd_interface overrides
	virtual void clock_write(int state) override;

private:
	bool const clock_held(unsigned usec) { return (machine().time() - m_clock_changed) >= attotime::from_usec(usec); }

	void resume();
	TIMER_CALLBACK_MEMBER(serial);
	void command(u8 const command);

	TIMER_CALLBACK_MEMBER(sample);
	void defaults();
	void update();

	required_ioport m_port_x_axis;
	required_ioport m_port_y_axis;
	required_ioport m_port_buttons;

	emu_timer *m_serial;
	emu_timer *m_sample;

	attotime m_clock_changed;

	// serial state
	enum serial_state : unsigned
	{
		IDLE,
		COMMAND,

		RX_START,
		RX_CLOCK_LO0,
		RX_CLOCK_LO1,
		RX_CLOCK_HI0,
		RX_CLOCK_HI1,

		TX_START,
		TX_CLOCK_LO0,
		TX_CLOCK_LO1,
		TX_CLOCK_HI0,
		TX_CLOCK_HI1,
	}
	m_state;
	unsigned m_bit;

	// setting state
	enum mode_mask : u8
	{
		SCALE  = 0x10, // 2:1 scaling
		ENABLE = 0x20, // data reporting
		REMOTE = 0x40,
		WRAP   = 0x80,
	};
	u8 m_mode;
	u8 m_sample_rate;
	u8 m_resolution;

	// tx/rx state
	unsigned m_rx_len;
	u8 m_rx_buf[2];
	unsigned m_tx_len;
	unsigned m_tx_pos;
	u8 m_tx_buf[4];
	u8 m_data;
	unsigned m_parity;

	// mouse state
	s16 m_mouse_x;
	s16 m_mouse_y;
	u8 m_mouse_b;
};

// device type definition
DECLARE_DEVICE_TYPE(HLE_PS2_MOUSE, hle_ps2_mouse_device)

#endif // MAME_BUS_HLE_PS2_MOUSE_H
