// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************

    CD-i Mono-I SLAVE MCU HLE
    -------------------

*******************************************************************************

STATUS:
- Just enough for the Mono-I CD-i board to work somewhat properly.

TODO:
- Proper LLE.

*******************************************************************************/

#ifndef MAME_PHILIPS_CDISLAVEHLE_H
#define MAME_PHILIPS_CDISLAVEHLE_H

#pragma once

#include "sound/dmadac.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cdislave_hle_device

class cdislave_hle_device : public device_t
{
public:
	// construction/destruction
	cdislave_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto int_callback() { return m_int_callback.bind(); }
	auto read_mousex() { return m_read_mousex.bind(); }
	auto read_mousey() { return m_read_mousey.bind(); }
	auto read_mousebtn() { return m_read_mousebtn.bind(); }

	uint8_t* get_lcd_state() { return m_lcd_state; }

	uint16_t slave_r(offs_t offset);
	void slave_w(offs_t offset, uint16_t data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// internal callbacks
	TIMER_CALLBACK_MEMBER( trigger_readback_int );
	TIMER_CALLBACK_MEMBER( poll_inputs );

private:
	void prepare_readback(const attotime &delay, uint8_t channel, uint8_t count, uint8_t data0, uint8_t data1, uint8_t data2, uint8_t data3, uint8_t cmd);
	void set_mouse_position();

	devcb_write_line m_int_callback;
	devcb_read16 m_read_mousex;
	devcb_read16 m_read_mousey;
	devcb_read8 m_read_mousebtn;

	required_device_array<dmadac_sound_device, 2> m_dmadac;

	struct channel_state
	{
		uint8_t m_out_buf[4];
		uint8_t m_out_index;
		uint8_t m_out_count;
		uint8_t m_out_cmd;
	};

	channel_state m_channel[4];
	emu_timer *m_interrupt_timer;
	emu_timer *m_input_poll_timer;

	uint8_t m_in_buf[17];
	uint8_t m_in_index;
	uint8_t m_in_count;

	uint8_t m_polling_active;

	uint8_t m_xbus_interrupt_enable;

	uint8_t m_lcd_state[16];

	uint16_t m_input_mouse_x;
	uint16_t m_input_mouse_y;
	uint8_t m_input_mouse_btn;

	int16_t m_device_mouse_x;
	int16_t m_device_mouse_y;
};


// device type definition
DECLARE_DEVICE_TYPE(CDI_SLAVE_HLE, cdislave_hle_device)

#endif // MAME_PHILIPS_CDISLAVEHLE_H
